#include "CollisionWorldThreadLogic.h"

#include "btBulletDynamicsCommon.h"

#include "Logger/Log.h"

namespace AV{
    CollisionWorldThreadLogic::CollisionWorldThreadLogic(uint8 worldId)
        : mWorldId(worldId) {

    }

    CollisionWorldThreadLogic::~CollisionWorldThreadLogic(){

    }

    void CollisionWorldThreadLogic::updateWorld(){
        checkInputBuffers();

        mPhysicsWorld->performDiscreteCollisionDetection();

        updateOutputBuffer();
    }

    void CollisionWorldThreadLogic::checkInputBuffers(){
        _processObjectInputBuffer();
    }

    void CollisionWorldThreadLogic::updateOutputBuffer(){
        const int numManifolds = mCollisionDispatcher->getNumManifolds();
        //AV_ERROR(numManifolds);
        for(int i = 0; i < numManifolds; i++){
            btPersistentManifold* contactManifold = mCollisionDispatcher->getManifoldByIndexInternal(i);
            btCollisionObject* obA = (btCollisionObject*)contactManifold->getBody0();
            btCollisionObject* obB = (btCollisionObject*)contactManifold->getBody1();

            //Process what about the objects needs to be relayed to the main thread.
        }
    }

    void CollisionWorldThreadLogic::_processObjectInputBuffer(){
        std::unique_lock<std::mutex> inputBufferLock(objectInputBufferMutex);
        if(inputObjectCommandBuffer.empty()) return;

        for(const ObjectCommandBufferEntry& entry : inputObjectCommandBuffer){
            if(entry.type == ObjectCommandType::COMMAND_TYPE_NONE) continue;

            btCollisionObject* b = entry.body;
            switch(entry.type){
                case ObjectCommandType::COMMAND_TYPE_ADD_OBJECT:
                    mPhysicsWorld->addCollisionObject(b);
                    break;
                case ObjectCommandType::COMMAND_TYPE_REMOVE_OBJECT:
                    mPhysicsWorld->removeCollisionObject(b);
                    break;
                default:
                    break;
            }
        }

        inputObjectCommandBuffer.clear();
    }

    void CollisionWorldThreadLogic::constructWorld(){
        AV_INFO("Creating collision world.")
        mBroadphaseCollision = new btDbvtBroadphase();
        mCollisionWorldConfiguration = new btDefaultCollisionConfiguration();
        mCollisionDispatcher = new btCollisionDispatcher(mCollisionWorldConfiguration);

        mPhysicsWorld = new btCollisionWorld(mCollisionDispatcher, mBroadphaseCollision, mCollisionWorldConfiguration);
    }

    void CollisionWorldThreadLogic::destroyWorld(){
        AV_INFO("Destroying collision world.");
        delete mPhysicsWorld;

        delete mCollisionDispatcher;
        delete mCollisionWorldConfiguration;
        delete mBroadphaseCollision;

        mPhysicsWorld = 0;
        mWorldDestroyComplete = true;
    }
}
