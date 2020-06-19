#include "CollisionWorldThreadLogic.h"

#include "btBulletDynamicsCommon.h"
#include "World/Physics/Worlds/CollisionWorldUtils.h"

#include "Logger/Log.h"

namespace AV{

    CollisionWorldThreadLogic* _collisionWorld = 0;

    void CollisionWorldThreadLogic::_processStartedEndedCallback(btPersistentManifold* const& manifold, bool started){
        //If one of the shapes is a rigid body, assume this manifold was found in the dynamics world.
        if(manifold->getBody0()->getInternalType() == btCollisionObject::CO_RIGID_BODY) return;
        const btCollisionObject* obj0 = manifold->getBody0();
        const btCollisionObject* obj1 = manifold->getBody1();
        assert(obj1->getInternalType() == btCollisionObject::CO_COLLISION_OBJECT);

        if(!CollisionWorldUtils::shouldObjectsSendEvent(obj0->getUserIndex(), obj1->getUserIndex())) return;

        _collisionWorld->tempObjectEventBuffer.push_back({obj0, obj1, started ? CollisionObjectEvent::ENTER : CollisionObjectEvent::LEAVE});
    }

    //Callback functions for collision manifolds.
    void CollisionWorldThreadLogic::contactStartedCallback(btPersistentManifold* const& manifold){
        _processStartedEndedCallback(manifold, true);
    }

    void CollisionWorldThreadLogic::contactEndedCallback(btPersistentManifold* const& manifold){
        _processStartedEndedCallback(manifold, false);
    }

    //It seems like bullet doesn't use this returned value for anything.
    bool CollisionWorldThreadLogic::contactProcessedCallback(btManifoldPoint& cp, void* body0, void* body1){
        if(static_cast<btCollisionObject*>(body0)->getInternalType() == btCollisionObject::CO_RIGID_BODY) return false;
        assert(static_cast<btCollisionObject*>(body1)->getInternalType() == btCollisionObject::CO_COLLISION_OBJECT);
        //AV_ERROR("CONTACT Processed");

        //TODO I should profile with this function as well as just a loop through the world.
        //Calling a function like this for every world will be slow, even if it can do some checks and return immediately.
        //I'm worried the dynamics world particularly will be slow. Even though this is threaded and probably won't matter, I should avoid it anyway if I can.

        return true;
    }



    CollisionWorldThreadLogic::CollisionWorldThreadLogic(uint8 worldId)
        : mWorldId(worldId) {

        //Set the callbacks.
        //Unfortunately this applies to all of bullet, not specific to any one world (including dynamcis worlds).
        //As such I have to do some sanity checks after each manifold.
        gContactStartedCallback = &contactStartedCallback;
        gContactEndedCallback = &contactEndedCallback;
        gContactProcessedCallback = &contactProcessedCallback;

        //TODO in future this will be an array for each object. I can't do this until I have an id baked into the collision object.
        _collisionWorld = this;
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
        _processInputBuffer();
    }

    void CollisionWorldThreadLogic::updateOutputBuffer(){
        // const int numManifolds = mCollisionDispatcher->getNumManifolds();
        // int foundManifolds = 0;
        // for(int i = 0; i < numManifolds; i++){
        //     btPersistentManifold* contactManifold = mCollisionDispatcher->getManifoldByIndexInternal(i);
        //     btCollisionObject* obA = (btCollisionObject*)contactManifold->getBody0();
        //     btCollisionObject* obB = (btCollisionObject*)contactManifold->getBody1();
        //
        //     CollisionPackedInt dataA = obA->getUserIndex();
        //     CollisionPackedInt dataB = obA->getUserIndex();
        //
        //     //The two manifolds are of the same type, meaning there's nothing to check.
        //     //if(CollisionWorldUtils::doesPackedIntsHaveSameType(dataA, dataB)) continue;
        //
        //     //Process what about the objects needs to be relayed to the main thread.
        // }
        //AV_ERROR(foundManifolds);
        AV_ERROR(mCollisionDispatcher->getNumManifolds());

        //The temp one can be checked here as it is only ever read and written to on this thread.
        if(tempObjectEventBuffer.empty()) return;

        std::unique_lock<std::mutex> inputBufferLock(objectOutputBufferMutex);
        //Insert the values from the temp list into the actual list.
        outputObjectEventBuffer.insert(outputObjectEventBuffer.end(), tempObjectEventBuffer.begin(), tempObjectEventBuffer.end());

        tempObjectEventBuffer.clear();
    }

    void CollisionWorldThreadLogic::_processInputBuffer(){
        std::unique_lock<std::mutex> inputBufferLock(inputBufferMutex);
        if(inputCommandBuffer.empty()) return;

        for(const InputBufferEntry& entry : inputCommandBuffer){
            btCollisionObject* b = entry.object;
            switch(entry.type){
                case InputBufferType::COMMAND_TYPE_SET_POSITION:
                    b->getWorldTransform().setOrigin(entry.val);
                    break;
                default:
                    break;
            };
        }

        inputCommandBuffer.clear();
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
