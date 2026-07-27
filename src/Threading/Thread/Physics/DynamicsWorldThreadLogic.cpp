#include "DynamicsWorldThreadLogic.h"

#include "Logger/Log.h"
#include "btBulletDynamicsCommon.h"

#include "Physics/Worlds/DynamicsWorldMotionState.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"
#include "System/SystemSetup/SystemSettings.h"

#include "OgreVector3.h"

namespace AV{
    DynamicsWorldThreadLogic::DynamicsWorldThreadLogic()
        : PhysicsWorldThreadLogic() {
        DynamicsWorldMotionState::dynLogic = this;

        mWorldDestroyComplete = false;
    }

    void DynamicsWorldThreadLogic::updateWorld(){
        //AV_INFO("Updating dynamics world.");

        checkInputBuffers();

        btDiscreteDynamicsWorld* dynWorld = static_cast<btDiscreteDynamicsWorld*>(mPhysicsWorld);
        dynWorld->stepSimulation(1.0f / 60.0f, 10);

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
