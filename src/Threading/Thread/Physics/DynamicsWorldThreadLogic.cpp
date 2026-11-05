#include "DynamicsWorldThreadLogic.h"

#include "Logger/Log.h"
#include "btBulletDynamicsCommon.h"

#include "Physics/Worlds/DynamicsWorldMotionState.h"
#include "System/SystemSetup/SystemSettings.h"

#include "OgreVector3.h"

namespace AV{
    DynamicsWorldThreadLogic::DynamicsWorldThreadLogic()
        : PhysicsWorldThreadLogic() {
        DynamicsWorldMotionState::dynLogic = this;

        mWorldDestroyComplete = false;
    }

    void DynamicsWorldThreadLogic::updateWorld(float timeStep){
        //AV_INFO("Updating dynamics world.");

        checkInputBuffers();

        btDiscreteDynamicsWorld* dynWorld = static_cast<btDiscreteDynamicsWorld*>(mPhysicsWorld);
        //maxSubSteps of 0 puts bullet in variable timestep mode, where it advances by exactly
        //timeStep once and leaves its own internal accumulator alone. PhysicsThread already
        //accumulates and only ever passes a constant 1/PhysicsUpdateRate, so letting bullet
        //accumulate as well would just stack a second, drifting accumulator underneath ours.
        dynWorld->stepSimulation(timeStep, 0);

        updateOutputBuffer();
    }

    void DynamicsWorldThreadLogic::checkInputBuffers(){
        //Of the buffers, The input buffer is processed first.
        //The input buffer contains mostly set commands (position, orientation, velocity), while the object buffer contains admin commands like remove from world, or destroy.
        //They are separated because the object input buffer has to be policed to check there are no duplicated commands (i.e occasional for loop checks)
        //An example of a duplicated command would to be an add and remove command for the same entity.
        //The input buffer commands need no such checks.
        _processInputBuffer();
        _processObjectInputBuffer();
    }

    void DynamicsWorldThreadLogic::_processObjectInputBuffer(){
        std::unique_lock<std::mutex> inputBufferLock(objectInputBufferMutex);
        if(inputObjectCommandBuffer.size() <= 0) return;

        btDiscreteDynamicsWorld* dynamicsWorld = static_cast<btDiscreteDynamicsWorld*>(mPhysicsWorld);
        for(const ObjectCommandBufferEntry& entry : inputObjectCommandBuffer){
            if(entry.type == ObjectCommandType::COMMAND_TYPE_NONE) continue;

            btRigidBody* b = entry.body;
            switch(entry.type){
                case ObjectCommandType::COMMAND_TYPE_ADD_BODY:
                    dynamicsWorld->addRigidBody(b);
                    break;
                case ObjectCommandType::COMMAND_TYPE_REMOVE_BODY:
                    dynamicsWorld->removeRigidBody(b);
                    break;
                case ObjectCommandType::COMMAND_TYPE_DESTROY_BODY: {
                    if(b->isInWorld()){
                        dynamicsWorld->removeRigidBody(b);
                    }
                    std::unique_lock<std::mutex> outputDestructionLock(outputDestructionBufferMutex);
                    outputDestructionBuffer.push_back({b, ObjectDestructionType::DESTRUCTION_TYPE_BODY});
                    break;
                }
                default:{
                    break;
                }
            }
        }

        inputObjectCommandBuffer.clear();
    }

    void DynamicsWorldThreadLogic::_processInputBuffer(){
        std::unique_lock<std::mutex> inputBufferLock(inputBufferMutex);
        if(inputBuffer.size() <= 0) return;

        for(const InputBufferEntry& entry : inputBuffer){
            if(entry.type == InputBufferCommandType::COMMAND_TYPE_NONE) continue;

            btRigidBody* b = entry.body;
            switch(entry.type){
                case InputBufferCommandType::COMMAND_TYPE_SET_POSITION:{
                    b->getWorldTransform().setOrigin(entry.val);
                    b->activate();
                    break;
                }
                case InputBufferCommandType::COMMAND_TYPE_SET_LINEAR_FACTOR:{
                    b->setLinearFactor(entry.val);
                    break;
                }
                case InputBufferCommandType::COMMAND_TYPE_SET_LINEAR_VELOCITY:{
                    b->setLinearVelocity(entry.val);
                    b->activate();
                    break;
                }
                default:{
                    assert(false);
                }
            }
        }

        inputBuffer.clear();
    }

    void DynamicsWorldThreadLogic::_notifyBodyMoved(btRigidBody *body){
        mMovedBodies.push_back(body);
    }


    void DynamicsWorldThreadLogic::updateOutputBuffer(){
        std::unique_lock<std::mutex> outputLock(outputBufferMutex);

        outputBuffer.clear();

        for(btRigidBody* i : mMovedBodies){
            btTransform trans;
            i->getMotionState()->getWorldTransform(trans);
            //Here I duplicate the transform data, because otherwise it's a potential race condition.
            //The motion state should only be written or read by the physics thread.
            outputBuffer.push_back({i, trans.getOrigin(), trans.getRotation(), i->getLinearVelocity()});
        }

        mMovedBodies.clear();

        _collectImpacts();
    }

    void DynamicsWorldThreadLogic::_collectImpacts(){
        const float threshold = impactThreshold.load();
        if(threshold < 0.0f) return;

        const int numManifolds = mDispatcher->getNumManifolds();
        for(int i = 0; i < numManifolds; i++){
            const btPersistentManifold* manifold = mDispatcher->getManifoldByIndexInternal(i);
            btRigidBody* a = btRigidBody::upcast(const_cast<btCollisionObject*>(manifold->getBody0()));
            btRigidBody* b = btRigidBody::upcast(const_cast<btCollisionObject*>(manifold->getBody1()));

            //A sleeping pair isn't stepped, so its manifold still holds whatever the last step left in it.
            if(!a->isActive() && !b->isActive()) continue;

            //A new point isn't a hit on its own, as a sliding body makes them constantly. What decides it is how hard the
            //solver had to work on them, which bullet writes back to the point during the step.
            btScalar impulse = 0;
            const btManifoldPoint* strongest = 0;
            for(int p = 0; p < manifold->getNumContacts(); p++){
                const btManifoldPoint& point = manifold->getContactPoint(p);
                if(point.getLifeTime() > 1) continue;

                impulse += point.getAppliedImpulse();
                if(!strongest || point.getAppliedImpulse() > strongest->getAppliedImpulse()) strongest = &point;
            }
            if(!strongest) continue;

            //The change in speed of the lighter body, so that the same number means the same hit at any mass. A contact
            //just resting reads one step's worth of gravity.
            const float strength = impulse * btMax(a->getInvMass(), b->getInvMass());
            if(strength <= threshold) continue;

            outputImpactBuffer.push_back({a, b, strongest->getPositionWorldOnB(), strength});
        }
    }

    void DynamicsWorldThreadLogic::constructWorld(){
        AV_INFO("Creating dynamics world.")

        mCollisionConfiguration = new btDefaultCollisionConfiguration();
        mDispatcher = new btCollisionDispatcher(mCollisionConfiguration);
        mOverlappingPairCache = new btDbvtBroadphase();
        mSolver = new btSequentialImpulseConstraintSolver;
        btDiscreteDynamicsWorld* dynWorld = new btDiscreteDynamicsWorld(mDispatcher, mOverlappingPairCache, mSolver, mCollisionConfiguration);

        dynWorld->setGravity(btVector3(0, -9.8, 0));
        mPhysicsWorld = dynWorld;
    }

    void DynamicsWorldThreadLogic::destroyWorld(){
        AV_INFO("Destroying dynamics world.");

        delete mPhysicsWorld;
        delete mSolver;
        delete mOverlappingPairCache;
        delete mDispatcher;
        delete mCollisionConfiguration;

        mPhysicsWorld = 0;
        mWorldDestroyComplete = true;
    }
}
