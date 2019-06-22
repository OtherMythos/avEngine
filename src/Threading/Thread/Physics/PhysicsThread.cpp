#include "PhysicsThread.h"

#include "Logger/Log.h"
#include "World/Physics/PhysicsManager.h"
#include "World/Physics/Worlds/DynamicsWorld.h"
#include "DynamicsWorldThreadLogic.h"

#include "btBulletDynamicsCommon.h"

namespace AV{
    PhysicsThread::PhysicsThread()
        : mReady(false),
          mPhysicsManagerReady(false),
          mRunning(false),
          mWorldsShouldExist(false),
          mTimestepSync(0),
          mDynLogic(std::shared_ptr<DynamicsWorldThreadLogic>(new DynamicsWorldThreadLogic())) {

    }

    void PhysicsThread::run(){
        mRunning = true;

        std::unique_lock<std::mutex> runningLock(mRunningMutex);

        while(mRunning){
            //Check if the world needs destruction.
            mDynLogic->checkWorldConstructDestruct(mWorldsShouldExist);

            if(!mReady || !mPhysicsManagerReady){
                //This will notify us when something happens to the world, and the checks should be re-performed.
                cv.wait(runningLock);
                continue;
            }
            if(mTimestepSync > 0){
                //We have an update to process.
                mTimestepSync = 0;
                mDynLogic->updateWorld();
            }else{
                //If there is nothing to process wait for something to arrive.
                cv.wait(runningLock);
            }
        }
    }

    void PhysicsThread::shutdown(){
        mRunning = false;
        cv.notify_all();
    }

    void PhysicsThread::setReady(bool ready){
        //mReady is atomic, so no mutex is required.
        mReady = ready;
        cv.notify_all();
    }

    void PhysicsThread::scheduleWorldUpdate(int time){
        mTimestepSync += time;

        cv.notify_all();
    }

    void PhysicsThread::providePhysicsManager(std::shared_ptr<PhysicsManager> physicsManager){
        //The world is created

        mPhysicsManagerReady = true;

        //This function is called by the main thread.
        //Here the world would need to be flagged as created, but this cannot be performed by the main thread.
        //So a flag is set to tell the thread to create the world when an update tick happens.
        mWorldsShouldExist = true;

        //As this is done by the main thread no checks are necessary.
        physicsManager->getDynamicsWorld()->setDynamicsWorldThreadLogic(mDynLogic.get());
    }

    void PhysicsThread::removePhysicsManager(){
        //The world is destroyed

        mPhysicsManagerReady = false;
        mWorldsShouldExist = false;
    }
}
