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
        std::unique_lock<std::mutex> inputBufferLock(inputBufferMutex);

        _processObjectInputBuffer();
        _processInputBuffer();
    }

    void DynamicsWorldThreadLogic::_processObjectInputBuffer(){
        if(inputObjectCommandBuffer.size() <= 0) return;

        for(const objectCommandBufferEntry& entry : inputObjectCommandBuffer){
            if(entry.type == ObjectCommandType::COMMAND_TYPE_NONE) continue;

            switch(entry.type){
                case ObjectCommandType::COMMAND_TYPE_ADD:
                    mDynamicsWorld->addRigidBody(entry.body);
                    break;
                case ObjectCommandType::COMMAND_TYPE_REMOVE:
                    mDynamicsWorld->removeRigidBody(entry.body);
                    break;
            }
        }

        inputObjectCommandBuffer.clear();
    }

    void DynamicsWorldThreadLogic::_processInputBuffer(){
        if(inputBuffer.size() <= 0) return;

        //Do whatever processing on the input buffer here.

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
            outputBuffer.push_back({i, trans.getOrigin()});
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
        AV_INFO("Destroying dynamics world.")

        for (int i = mDynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
        {
            btCollisionObject* obj = mDynamicsWorld->getCollisionObjectArray()[i];
            btRigidBody* body = btRigidBody::upcast(obj);
            if (body && body->getMotionState())
            {
                delete body->getMotionState();
            }
            mDynamicsWorld->removeCollisionObject(obj);
            delete obj;
        }

        //delete collision shapes
        /*for (int j = 0; j < collisionShapes.size(); j++)
        {
            btCollisionShape* shape = collisionShapes[j];
            collisionShapes[j] = 0;
            delete shape;
        }*/

        delete mDynamicsWorld;
        delete mSolver;
        delete mOverlappingPairCache;
        delete mDispatcher;
        delete mCollisionConfiguration;

        mDynamicsWorld = 0;
    }
}
