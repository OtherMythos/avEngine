#include "CollisionWorldThreadLogic.h"

#include "btBulletDynamicsCommon.h"
#include "World/Physics/Worlds/CollisionWorldUtils.h"
#include "World/Physics/PhysicsBodyDestructor.h"

#include "Logger/Log.h"

namespace AV{

    CollisionWorldThreadLogic* _collisionWorld = 0;

    inline void orderSenderAndReceiver(int APackedInt, const btCollisionObject** a, const btCollisionObject** b){
        CollisionObjectType::CollisionObjectType obj0Type = CollisionWorldUtils::_readPackedIntType(APackedInt);
        if(obj0Type == CollisionObjectType::RECEIVER){
            const btCollisionObject* tmp = *a;
            *a = *b;
            *b = tmp;
        }
    }

    void CollisionWorldThreadLogic::_processStartedEndedCallback(btPersistentManifold* const& manifold, bool started){
        //If one of the shapes is a rigid body, assume this manifold was found in the dynamics world.
        if(manifold->getBody0()->getInternalType() == btCollisionObject::CO_RIGID_BODY) return;
        const btCollisionObject* obj0 = manifold->getBody0();
        const btCollisionObject* obj1 = manifold->getBody1();
        assert(obj1->getInternalType() == btCollisionObject::CO_COLLISION_OBJECT);

        if(!CollisionWorldUtils::shouldObjectsSendEvent(started ? CollisionObjectEventMask::ENTER : CollisionObjectEventMask::LEAVE, obj0->getUserIndex(), obj1->getUserIndex())) return;

        //When they're pushed, obj0 should always be a sender, so this function switches them if this isn't the case.
        orderSenderAndReceiver(obj0->getUserIndex(), &obj0, &obj1);
        assert(CollisionWorldUtils::_readPackedIntType(obj0->getUserIndex()) != CollisionObjectType::RECEIVER);
        assert(CollisionWorldUtils::_readPackedIntType(obj1->getUserIndex()) == CollisionObjectType::RECEIVER);

        _collisionWorld->tempObjectEventBuffer.push_back({obj0, obj1, started ? CollisionObjectEventMask::ENTER : CollisionObjectEventMask::LEAVE});
    }

    //Callback functions for collision manifolds.
    void CollisionWorldThreadLogic::contactStartedCallback(btPersistentManifold* const& manifold){
        _processStartedEndedCallback(manifold, true);
    }

    void CollisionWorldThreadLogic::contactEndedCallback(btPersistentManifold* const& manifold){
        _processStartedEndedCallback(manifold, false);
    }


    CollisionWorldThreadLogic::CollisionWorldThreadLogic(uint8 worldId)
        : mWorldId(worldId) {

        //Set the callbacks.
        //Unfortunately this applies to all of bullet, not specific to any one world (including dynamcis worlds).
        //As such I have to do some sanity checks after each manifold.
        gContactStartedCallback = &contactStartedCallback;
        gContactEndedCallback = &contactEndedCallback;

        //TODO in future this will be an array for each object. I can't do this until I have an id baked into the collision object.
        _collisionWorld = this;
        mWorldDestroyComplete = false;
    }

    CollisionWorldThreadLogic::~CollisionWorldThreadLogic(){

    }

    void CollisionWorldThreadLogic::updateWorld(){
        checkInputBuffers();

        mPhysicsWorld->performDiscreteCollisionDetection();

        updateOutputBuffer();
    }

    void CollisionWorldThreadLogic::checkInputBuffers(){
        if(worldShifted){
            _performOriginShift();
        }

        _processInputBuffer();
        _processObjectInputBuffer();
    }

    void CollisionWorldThreadLogic::updateOutputBuffer(){
        //Insert the values present each frame into this list.
        const int numManifolds = mCollisionDispatcher->getNumManifolds();
        for(int i = 0; i < numManifolds; i++){
            btPersistentManifold* contactManifold = mCollisionDispatcher->getManifoldByIndexInternal(i);
            const btCollisionObject* obj0 = (const btCollisionObject*)contactManifold->getBody0();
            const btCollisionObject* obj1 = (const btCollisionObject*)contactManifold->getBody1();

            if(!CollisionWorldUtils::shouldObjectsSendEvent(CollisionObjectEventMask::INSIDE, obj0->getUserIndex(), obj1->getUserIndex())) continue;

            //When they're pushed, obj0 should always be a sender, so this function switches them if this isn't the case.
            orderSenderAndReceiver(obj0->getUserIndex(), &obj0, &obj1);
            assert(CollisionWorldUtils::_readPackedIntType(obj0->getUserIndex()) != CollisionObjectType::RECEIVER);
            assert(CollisionWorldUtils::_readPackedIntType(obj1->getUserIndex()) == CollisionObjectType::RECEIVER);
            tempObjectEventBuffer.push_back({obj0, obj1, CollisionObjectEventMask::INSIDE});
        }

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
            if(entry.type == InputBufferType::COMMAND_TYPE_NONE) continue;

            btCollisionObject* b = entry.object;
            switch(entry.type){
                case InputBufferType::COMMAND_TYPE_SET_POSITION:
                    b->getWorldTransform().setOrigin(entry.val);
                    break;
                default:
                    assert(false);
            };
        }

        inputCommandBuffer.clear();
    }

    void CollisionWorldThreadLogic::_processObjectInputBuffer(){
        std::unique_lock<std::mutex> inputBufferLock(objectInputBufferMutex);
        if(inputObjectCommandBuffer.empty()) return;

        for(const ObjectCommandBufferEntry& entry : inputObjectCommandBuffer){
            if(entry.type == ObjectCommandType::COMMAND_TYPE_NONE) continue;

            btCollisionObject* b = entry.object;
            //TODO have an assert here that checks this body is of this world.
            switch(entry.type){
                case ObjectCommandType::COMMAND_TYPE_ADD_OBJECT: {
                    int userIdx = CollisionWorldUtils::produceThreadPackedInt(true);
                    b->setUserIndex2(userIdx);
                    mPhysicsWorld->addCollisionObject(b);
                    break;
                }
                case ObjectCommandType::COMMAND_TYPE_REMOVE_OBJECT: {
                    int userIdx = CollisionWorldUtils::produceThreadPackedInt(false);
                    b->setUserIndex2(userIdx);
                    mPhysicsWorld->removeCollisionObject(b);
                    break;
                }
                case ObjectCommandType::COMMAND_TYPE_DESTROY_OBJECT: {
                    bool objectInWorld = CollisionWorldUtils::_readPackedIntInWorld(b->getUserIndex2());
                    if(objectInWorld){
                        mPhysicsWorld->removeCollisionObject(b);
                    }
                    std::unique_lock<std::mutex> outputDestructionLock(PhysicsBodyDestructor::mCollisionObjectDestructionBufferMutex);
                    PhysicsBodyDestructor::mCollisonObjectDestructionBuffer.push_back({b, PhysicsBodyDestructor::CollisionObjectDestructionType::DESTRUCTION_TYPE_OBJECT});
                    break;
                }
                case ObjectCommandType::COMMAND_TYPE_ADD_CHUNK: {
                    PhysicsTypes::CollisionObjectsVector vec = reinterpret_cast<PhysicsTypes::CollisionObjectsVector>(b);
                    SlotPosition chunkPos(entry.x, entry.y);
                    btVector3 originPos = chunkPos.toBulletWithOrigin(worldOriginChangeNewPosition);

                    for(btCollisionObject* obj : *vec){
                        if(CollisionWorldUtils::_readPackedIntWorldId(obj->getUserIndex()) == mWorldId){
                            obj->getWorldTransform().setOrigin(originPos + obj->getWorldTransform().getOrigin());
                            mPhysicsWorld->addCollisionObject(obj);
                        }
                    }
                    break;
                }
                default:
                    assert(false);
                    break;
            }
        }

        inputObjectCommandBuffer.clear();
    }

    void CollisionWorldThreadLogic::_performOriginShift(){
        const btCollisionObjectArray& objs = mPhysicsWorld->getCollisionObjectArray();
        for(int i = 0; i < mPhysicsWorld->getNumCollisionObjects(); i++){
            btCollisionObject* obj = objs[i];

            btVector3 currentPos = obj->getWorldTransform().getOrigin();
            obj->getWorldTransform().setOrigin(currentPos - worldOriginChangeOffset);
        }

        //Reset the offset once the value has been read.
        worldOriginChangeOffset = btVector3();
        worldShifted = false;
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

        tempObjectEventBuffer.clear();
        {
            std::unique_lock<std::mutex> inputBufferLock(objectOutputBufferMutex);
            outputObjectEventBuffer.clear();
        }

        delete mPhysicsWorld;

        delete mCollisionDispatcher;
        delete mCollisionWorldConfiguration;
        delete mBroadphaseCollision;

        mPhysicsWorld = 0;
        mWorldDestroyComplete = true;
    }
}
