#include "DynamicsWorldThreadLogic.h"

#include "Logger/Log.h"
#include "btBulletDynamicsCommon.h"

#include "World/Physics/Worlds/DynamicsWorldMotionState.h"

namespace AV{
    DynamicsWorldThreadLogic::DynamicsWorldThreadLogic(){
        DynamicsWorldMotionState::dynLogic = this;
    }

    void DynamicsWorldThreadLogic::updateWorld(){
        //AV_INFO("Updating dynamics world.");

        checkInputBuffers();

        mDynamicsWorld->stepSimulation(1.0f / 60.0f, 10);

        updateOutputBuffer();
    }

    void DynamicsWorldThreadLogic::checkInputBuffers(){
        //Check if the world was shifted first.
        //This was put here to avoid an issue I was seeing, where setting the origin, and then setting an entity position would cause incorrect positons.
        //Position sets were always performed before the origin shift, which means the body could have had the new position set (accounting for the new origin), and then the offset later.
        //Performing the shift first means there is no conflict.
        if(worldShifted){
            _performOriginShift(worldOriginChangeOffset);
        }

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

        for(const objectCommandBufferEntry& entry : inputObjectCommandBuffer){
            if(entry.type == ObjectCommandType::COMMAND_TYPE_NONE) continue;

            btRigidBody* b = entry.body;
            switch(entry.type){
                case ObjectCommandType::COMMAND_TYPE_ADD:
                    mDynamicsWorld->addRigidBody(b);
                    break;
                case ObjectCommandType::COMMAND_TYPE_REMOVE:
                    mDynamicsWorld->removeRigidBody(b);
                    break;
                case ObjectCommandType::COMMAND_TYPE_DESTROY: {
                    if(b->isInWorld()){
                        mDynamicsWorld->removeRigidBody(b);
                    }
                    std::unique_lock<std::mutex> outputDestructionLock(outputDestructionBufferMutex);
                    outputDestructionBuffer.push_back({b, ObjectDestructionType::DESTRUCTION_TYPE_BODY});
                    break;
                }
                case ObjectCommandType::COMMAND_TYPE_ADD_CHUNK: {
                    std::vector<btRigidBody*>* vec = reinterpret_cast<std::vector<btRigidBody*>*>(b);
                    for(btRigidBody* bdy : *vec){
                        mDynamicsWorld->addRigidBody(bdy);
                    }
                    break;
                }
                case ObjectCommandType::COMMAND_TYPE_REMOVE_CHUNK: {
                    std::vector<btRigidBody*>* vec = reinterpret_cast<std::vector<btRigidBody*>*>(b);
                    for(btRigidBody* bdy : *vec){
                        mDynamicsWorld->removeRigidBody(bdy);
                    }
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

        for(const inputBufferEntry& entry : inputBuffer){
            if(entry.type == InputBufferCommandType::COMMAND_TYPE_NONE) continue;

            btRigidBody* b = entry.body;
            switch(entry.type){
                case InputBufferCommandType::COMMAND_TYPE_SET_POSITION:{
                    b->getWorldTransform().setOrigin(entry.val);
                    b->activate();
                    break;
                }
            }
        }

        inputBuffer.clear();
    }

    void DynamicsWorldThreadLogic::_notifyBodyMoved(btRigidBody *body){
        mMovedBodies.push_back(body);
    }

    void DynamicsWorldThreadLogic::_performOriginShift(btVector3 offset){
        const btCollisionObjectArray& objs = mDynamicsWorld->getCollisionObjectArray();
        for(int i = 0; i < mDynamicsWorld->getNumCollisionObjects(); i++){
            btCollisionObject* obj = objs[i];
            btRigidBody* body = btRigidBody::upcast(obj);

            btVector3 currentPos = body->getWorldTransform().getOrigin();
            body->getWorldTransform().setOrigin(currentPos - offset);
        }

        //Reset the offset once the value has been read.
        worldOriginChangeOffset = btVector3();
        worldShifted = false;
    }

    void DynamicsWorldThreadLogic::updateOutputBuffer(){
        std::unique_lock<std::mutex> outputLock(outputBufferMutex);

        outputBuffer.clear();

        for(btRigidBody* i : mMovedBodies){
            btTransform trans;
            i->getMotionState()->getWorldTransform(trans);
            //Here I duplicate the transform data, because otherwise it's a potential race condition.
            //The motion state should only be written or read by the physics thread.
            outputBuffer.push_back({i, trans.getOrigin(), trans.getRotation()});
        }

        mMovedBodies.clear();
    }

    void DynamicsWorldThreadLogic::checkWorldConstructDestruct(bool worldShouldExist){
        if(worldShouldExist && !mDynamicsWorld){
            constructWorld();
        }
        if(!worldShouldExist && mDynamicsWorld){
            destroyWorld();
        }
    }

    void DynamicsWorldThreadLogic::constructWorld(){
        AV_INFO("Creating dynamics world.")

        mCollisionConfiguration = new btDefaultCollisionConfiguration();
    	mDispatcher = new btCollisionDispatcher(mCollisionConfiguration);
        mOverlappingPairCache = new btDbvtBroadphase();
    	mSolver = new btSequentialImpulseConstraintSolver;
        mDynamicsWorld = new btDiscreteDynamicsWorld(mDispatcher, mOverlappingPairCache, mSolver, mCollisionConfiguration);

        mDynamicsWorld->setGravity(btVector3(0, -9.8, 0));
    }

    void DynamicsWorldThreadLogic::destroyWorld(){
        AV_INFO("Destroying dynamics world.");

        delete mDynamicsWorld;
        delete mSolver;
        delete mOverlappingPairCache;
        delete mDispatcher;
        delete mCollisionConfiguration;

        mDynamicsWorld = 0;
    }
}
