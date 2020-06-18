#include "CollisionWorldThreadLogic.h"

#include "btBulletDynamicsCommon.h"
#include "World/Physics/Worlds/CollisionWorldUtils.h"

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
        int foundManifolds = 0;
        for(int i = 0; i < numManifolds; i++){
            btPersistentManifold* contactManifold = mCollisionDispatcher->getManifoldByIndexInternal(i);
            btCollisionObject* obA = (btCollisionObject*)contactManifold->getBody0();
            btCollisionObject* obB = (btCollisionObject*)contactManifold->getBody1();

            CollisionPackedInt dataA = obA->getUserIndex();
            CollisionPackedInt dataB = obA->getUserIndex();

            //The two manifolds are of the same type, meaning there's nothing to check.
            if(CollisionWorldUtils::doesPackedIntsHaveSameType(dataA, dataB)) continue;

            //Process what about the objects needs to be relayed to the main thread.
        }
        AV_ERROR(foundManifolds);
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
