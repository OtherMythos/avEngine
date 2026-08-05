#include "PhysicsThread.h"
#include <cmath>

#include "Logger/Log.h"
#include "World/Physics/PhysicsManager.h"
#include "World/Physics/Worlds/DynamicsWorld.h"
#include "World/Physics/Worlds/CollisionWorld.h"
#include "World/Physics/PhysicsBodyDestructor.h"
#include "DynamicsWorldThreadLogic.h"
#include "CollisionWorldThreadLogic.h"
#include "System/SystemSetup/SystemSettings.h"

#include "BulletCollision/NarrowPhaseCollision/btPersistentManifold.h"

#include "btBulletDynamicsCommon.h"
#include "Logger/Log.h"

#include "System/Pause/PauseState.h"

namespace AV{
    namespace{
        //See the comment at the accumulator loop in run().
        const int64 STEP_TOLERANCE_NS = 1000;
    }

    PhysicsThread::PhysicsThread()
        : mReady(false),
          mPhysicsManagerReady(false),
          mWorldsShouldExist(false),
          mCurrentWorldVersion(0),
          mActiveCollisionWorlds(SystemSettings::getNumCollisionWorlds()) {

        _initialiseCollisionWorldSlots();
        for(uint8 i = 0; i < mActiveCollisionWorlds; i++){
            std::shared_ptr<CollisionWorldThreadLogic> c = std::make_shared<CollisionWorldThreadLogic>(i);
            PhysicsBodyDestructor::setCollisionWorldThreadLogic(i, c.get());
            mCollisionWorlds[i] = c;
        }
        if(SystemSettings::getDynamicPhysicsDisabled()) mDynLogic = 0;
        else{
            std::shared_ptr<DynamicsWorldThreadLogic> d = std::make_shared<DynamicsWorldThreadLogic>();
            //The destructor is destroyed on engine shutdown, so this doesn't need to be set on world restart.
            PhysicsBodyDestructor::setDynamicsWorldThreadLogic(d.get());
            mDynLogic = d;
        }
    }

    PhysicsThread::PhysicsThread(std::shared_ptr<PhysicsWorldThreadLogic> dynLogic, std::vector<std::shared_ptr<PhysicsWorldThreadLogic>> collisionWorlds)
        : mReady(false),
          mPhysicsManagerReady(false),
          mWorldsShouldExist(false),
          mCurrentWorldVersion(0),
          mActiveCollisionWorlds(static_cast<uint8>(collisionWorlds.size())) {

        assert(collisionWorlds.size() <= MAX_COLLISION_WORLDS);

        _initialiseCollisionWorldSlots();
        for(uint8 i = 0; i < mActiveCollisionWorlds; i++){
            mCollisionWorlds[i] = collisionWorlds[i];
        }
        mDynLogic = dynLogic;
    }

    void PhysicsThread::_initialiseCollisionWorldSlots(){
        for(int i = 0; i < MAX_COLLISION_WORLDS; i++){
            mCollisionWorlds[i].reset();
        }
    }

    void PhysicsThread::_initTimestepIfNeeded(){
        if(mTimestepNs != 0) return;

        const int rate = SystemSettings::getPhysicsUpdateRate();
        //Rounded rather than truncated, and matched by the rounding in ThreadManager, so that a
        //delta of 1/60 and a timestep of 1/60 are the same number of nanoseconds. Truncating both
        //leaves the accumulator a nanosecond short every update, which delays steps and drifts.
        mTimestepNs = llround(1000000000.0 / static_cast<double>(rate > 0 ? rate : 60));
        mMaxStepsPerUpdate = SystemSettings::getMaxPhysicsStepsPerUpdate();
        if(mMaxStepsPerUpdate <= 0) mMaxStepsPerUpdate = 1;
    }

    PhysicsThread::StepAcknowledger::~StepAcknowledger(){
        if(!mHaveStep) return;

        {
            std::lock_guard<std::mutex> lock(mParent->mStateMutex);
            mParent->mCompletedStep = mTarget;
        }
        mParent->mDoneCV.notify_all();
    }

    void PhysicsThread::run(){
        std::unique_lock<std::mutex> lock(mStateMutex);
        mRunning = true;

        while(true){
            mWorkCV.wait(lock, [this]{
                return !mRunning || mWorldStateDirty || mScheduledStep > mCompletedStep;
            });
            if(!mRunning) break;

            const bool worldDirty = mWorldStateDirty;
            mWorldStateDirty = false;

            const bool haveStep = mScheduledStep > mCompletedStep;
            const uint64 target = mScheduledStep;
            const int64 delta = mPendingDeltaNs;
            mPendingDeltaNs = 0;

            const bool ready = mReady && mPhysicsManagerReady;
            const bool shouldExist = mWorldsShouldExist;
            const int version = mCurrentWorldVersion;

            lock.unlock();
            {
                //Acknowledges on every exit from this scope, so no branch below can strand the
                //main thread in waitForScheduledStep.
                StepAcknowledger ack(this, haveStep, target);

                //Check if the world needs construction or destruction.
                //TODO I could probably reduce this down to a single function call for all objects.
                if(worldDirty || haveStep){
                    if(mDynLogic) mDynLogic->checkWorldConstructDestruct(shouldExist, version);
                    for(int i = 0; i < mActiveCollisionWorlds; i++){
                        mCollisionWorlds[i]->checkWorldConstructDestruct(shouldExist, version);
                    }
                }

                const uint32 pauseMask = PauseState::getMask();
                const bool canStep = haveStep && ready && (pauseMask & PAUSE_TYPE_PHYSICS) == 0;

                if(!canStep){
                    //Never bank time across a pause, or unpausing would fire a burst of steps.
                    mAccumulatorNs = 0;
                }else{
                    _initTimestepIfNeeded();

                    mAccumulatorNs += delta;
                    int steps = 0;
                    //A rate of 60 is 16666666.67ns, which no integer can hold, so N timesteps
                    //never exactly equal one delta however the two are rounded - at 30 into 60 the
                    //second step would come up a nanosecond short and never run. The tolerance is
                    //orders of magnitude above that error and orders below the smallest legal
                    //timestep (4.17ms at the 240 rate cap), so it can absorb the representation
                    //error without ever manufacturing a step.
                    while(mAccumulatorNs + STEP_TOLERANCE_NS >= mTimestepNs && steps < mMaxStepsPerUpdate){
                        _stepWorlds(static_cast<float>(static_cast<double>(mTimestepNs) / 1000000000.0), pauseMask);
                        mAccumulatorNs -= mTimestepNs;
                        steps++;
                    }
                    //Hit the cap, so we are behind. Drop the remainder rather than trying to catch
                    //up, which would only fall further behind.
                    if(steps >= mMaxStepsPerUpdate) mAccumulatorNs = 0;
                }
            }
            lock.lock();
        }

        //Release anyone blocked waiting on a step which will now never run.
        mCompletedStep = mScheduledStep;
        lock.unlock();
        mDoneCV.notify_all();
    }

    void PhysicsThread::_stepWorlds(float timeStep, uint32 pauseMask){
        if(mDynLogic && (pauseMask & PAUSE_TYPE_PHYSICS_DYNAMICS) == 0) mDynLogic->updateWorld(timeStep);

        if((pauseMask & PAUSE_TYPE_PHYSICS_COLLISION) == 0){
            for(int i = 0; i < mActiveCollisionWorlds; i++){
                if((pauseMask & (PAUSE_TYPE_PHYSICS_COLLISION0 << i)) == 0)
                    mCollisionWorlds[i]->updateWorld(timeStep);
            }
        }
    }

    void PhysicsThread::shutdown(){
        {
            std::lock_guard<std::mutex> lock(mStateMutex);
            mRunning = false;
        }
        mWorkCV.notify_all();
        //Also released here, in case the main thread is blocked on a step that will never run.
        mDoneCV.notify_all();
    }

    void PhysicsThread::setReady(bool ready){
        {
            //Taken so the flag and the notify cannot land between the physics thread evaluating
            //its predicate and going to sleep, which would lose the wakeup.
            std::lock_guard<std::mutex> lock(mStateMutex);
            mReady = ready;
            mWorldStateDirty = true;
        }
        mWorkCV.notify_all();
    }

    void PhysicsThread::scheduleStep(int64 deltaNs){
        {
            std::lock_guard<std::mutex> lock(mStateMutex);
            mPendingDeltaNs += deltaNs;
            mScheduledStep++;
        }
        mWorkCV.notify_all();
    }

    void PhysicsThread::waitForScheduledStep(){
        std::unique_lock<std::mutex> lock(mStateMutex);
        mDoneCV.wait(lock, [this]{
            return !mRunning || mCompletedStep >= mScheduledStep;
        });
    }

    void PhysicsThread::notifyWorldCreation(std::shared_ptr<PhysicsManager> physicsManager){
        //The world is created

        mPhysicsManagerReady = true;

        mCurrentWorldVersion++;

        //This function is called by the main thread.
        //Here the world would need to be flagged as created, but this cannot be performed by the main thread.
        //So a flag is set to tell the thread to create the world when an update tick happens.
        mWorldsShouldExist = true;

        //As this is done by the main thread no mutex checks are necessary.
        //The members are held as the base type so the thread can be unit tested with fakes; these
        //are the only two places which need the concrete type back.
        if(!SystemSettings::getDynamicPhysicsDisabled()){
            physicsManager->getDynamicsWorld()->setDynamicsWorldThreadLogic(static_cast<DynamicsWorldThreadLogic*>(mDynLogic.get()));
        }

        for(uint8 i = 0; i < mActiveCollisionWorlds; i++){
            physicsManager->getCollisionWorld(i)->setCollisionWorldThreadLogic(static_cast<CollisionWorldThreadLogic*>(mCollisionWorlds[i].get()));
        }
    }

    void PhysicsThread::notifyWorldDestruction(){
        //The world is destroyed

        mPhysicsManagerReady = false;
        mWorldsShouldExist = false;
    }
}
