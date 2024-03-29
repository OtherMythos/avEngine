#include "PhysicsBodyDestructor.h"

#include "PhysicsBodyConstructor.h"
#include "PhysicsMetaDataManager.h"

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

#include "System/BaseSingleton.h"
#include "World/Slot/Chunk/TerrainManager.h"
#include "World/Physics/Worlds/CollisionWorldUtils.h"

#include "Threading/Thread/Physics/DynamicsWorldThreadLogic.h"
#include "Threading/Thread/Physics/CollisionWorldThreadLogic.h"
#include "World/Physics/Worlds/DynamicsWorldMotionState.h"

#include "Event/EventDispatcher.h"
#include "Event/Events/WorldEvent.h"

#ifdef DEBUGGING_TOOLS
    #include "World/WorldSingleton.h"
    #include "World/Developer/MeshVisualiser.h"
#endif

namespace AV{

    DynamicsWorldThreadLogic* PhysicsBodyDestructor::mDynLogic = 0;
    CollisionWorldThreadLogic* PhysicsBodyDestructor::mCollisionLogic[MAX_COLLISION_WORLDS];
    std::set<btRigidBody*> PhysicsBodyDestructor::mPendingBodies;
    std::set<btCollisionShape*> PhysicsBodyDestructor::mPendingShapes;
    std::set<btCollisionObject*> PhysicsBodyDestructor::mPendingCollisionObjects;
    bool PhysicsBodyDestructor::mWorldRecentlyDestroyed = false;
    bool PhysicsBodyDestructor::mWorldDestructionPending = false;

    std::vector<PhysicsBodyDestructor::CollisionOutputDestructionBufferEntry> PhysicsBodyDestructor::mCollisonObjectDestructionBuffer;
    std::mutex PhysicsBodyDestructor::mCollisionObjectDestructionBufferMutex;


    void PhysicsBodyDestructor::setup(){
        EventDispatcher::subscribeStatic(EventType::World, AV_BIND_STATIC(PhysicsBodyDestructor::worldEventReceiver));
    }

    void PhysicsBodyDestructor::shutdown(){
        //The engine is shutting down, so any bodies pending for destruction can now just be destroyed.
        _clearState();
    }

    void PhysicsBodyDestructor::_clearState(){
        for(btRigidBody* b : mPendingBodies){
            _destroyRigidBody(b);
        }
        for(btCollisionShape* s : mPendingShapes){
            //delete s;
            _destroyCollisionShape(s);
        }
        for(btCollisionObject* o : mPendingCollisionObjects){
            _destroyCollisionObject(o);
        }
        mPendingBodies.clear();
        mPendingShapes.clear();
        mPendingCollisionObjects.clear();
    }

    void PhysicsBodyDestructor::destroyTerrainBody(btRigidBody* bdy){
        //These are read only values, so there should be no thread-safety issues getting them.
        assert(bdy->getCollisionShape());
        btCollisionShape* shape = bdy->getCollisionShape();
        assert(shape->getShapeType() == TERRAIN_SHAPE_PROXYTYPE);
        assert(mDynLogic); //Terrain can't exist without the world.

        mPendingBodies.insert(bdy);
        mPendingShapes.insert(shape);

        std::unique_lock<std::mutex> inputBufferLock(mDynLogic->objectInputBufferMutex);
        mDynLogic->inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_DESTROY_BODY, bdy});
    }

    void PhysicsBodyDestructor::destroyRigidBody(btRigidBody* bdy){

        if(mDynLogic){
            std::unique_lock<std::mutex> inputBufferLock(mDynLogic->objectInputBufferMutex);

            //Request the body for destruction.
            mDynLogic->inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_DESTROY_BODY, bdy});

            mPendingBodies.insert(bdy);
        }else{
            //If there is no dynamic logic, we can assume there's no world. So destruction can happen immediately.

            _destroyRigidBody(bdy);
            //No need to clear the pending list, because this body is being destroyed immediately it would never make it in there anyway.
        }
    }

    void PhysicsBodyDestructor::destroyCollisionObject(btCollisionObject* object){
        uint8 targetId = CollisionWorldUtils::_readPackedIntWorldId(object->getUserIndex());

        CollisionWorldThreadLogic* targetLogic = mCollisionLogic[targetId];
        if(targetLogic){
            std::unique_lock<std::mutex> inputBufferLock(targetLogic->objectInputBufferMutex);

            //Request the object for destruction.
            targetLogic->inputObjectCommandBuffer.push_back({CollisionWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_DESTROY_OBJECT, object});

            mPendingCollisionObjects.insert(object);
        }else{
            //Likely there's no world.
            _destroyCollisionObject(object);
        }
    }

    void PhysicsBodyDestructor::destroyPhysicsWorldChunk(PhysicsTypes::PhysicsChunkEntry chunk){
        //Deleting shapes.
        //chunk.first is a vector of shared pointers, so calling this will destroy them as well.
        chunk.first->clear();
        //Delete the actual vector.
        delete chunk.first;

        for(btRigidBody* bdy : *(chunk.second) ){
            destroyRigidBody(bdy);
        }
        //Destroy the other vector.
        delete chunk.second;

        #ifdef DEBUGGING_TOOLS
            World* w = WorldSingleton::getWorld();
            assert(w);
            //The pointers are used for id, not actually read from.
            //Even though they've been destroyed, this is safe.
            w->getMeshVisualiser()->destroyPhysicsChunk(chunk);
        #endif
    }

    void PhysicsBodyDestructor::destroyCollisionShape(btCollisionShape *shape){

        //No mDynLogic means there is no world, so just delete the shapes.
        if(mDynLogic && _shapeEverAttached(shape)){
            mPendingShapes.insert(shape);
        }else{
            //If the shape was never attached to anything, we can just delete it.
            //If the world doesn't exist we can also just delete it.
            //delete shape;
            _destroyCollisionShape(shape);
        }
    }

    bool PhysicsBodyDestructor::_shapeEverAttached(btCollisionShape *shape){
        //Take the shape pointer as a 32 bit number, and take the final bit of that to indicate whether a shape was ever attached.
        return ((uintptr_t)shape->getUserPointer() & 0x80000000) >> 31;
    }

    void PhysicsBodyDestructor::update(){
        if(!mDynLogic) return; //Nothing to check

        _checkWorldDestructionReceipt();

        //Check the output buffer for bodies that can be destroyed.
        _checkDynamicsWorldDestructables();
        //Check the collision world output buffer.
        _checkCollisionWorldDestructables();

        //The system will check as part of the update whether anything has to be performed for this.
        //As this is the end of the function, it's checks have finished and this can be set to false.
        mWorldRecentlyDestroyed = false;
    }

    void PhysicsBodyDestructor::_checkCollisionWorldDestructables(){
        std::unique_lock<std::mutex> outputDestructionLock(mCollisionObjectDestructionBufferMutex);

        if(mWorldRecentlyDestroyed){
            mCollisonObjectDestructionBuffer.clear();
            return;
        }

        for(const CollisionOutputDestructionBufferEntry& entry : mCollisonObjectDestructionBuffer){
            switch(entry.type){
                case CollisionObjectDestructionType::DESTRUCTION_TYPE_OBJECT:{
                    btCollisionShape* s = entry.obj->getCollisionShape();
                    if(mPendingShapes.find(s) != mPendingShapes.end()){
                        //delete s;
                        _destroyCollisionShape(s);
                        mPendingShapes.erase(s);
                    }
                    _destroyCollisionObject(entry.obj);
                    mPendingCollisionObjects.erase(entry.obj);
                    break;
                }
                default:
                    break;
            }
        }

        mCollisonObjectDestructionBuffer.clear();
    }

    void PhysicsBodyDestructor::_checkDynamicsWorldDestructables(){
        std::unique_lock<std::mutex> outputDestructionLock(mDynLogic->outputDestructionBufferMutex);

        //If the world was destroyed last frame any pending shapes should already have been destroyed.
        if(mWorldRecentlyDestroyed){
            mDynLogic->outputDestructionBuffer.clear();
            return;
        }

        for(const DynamicsWorldThreadLogic::OutputDestructionBufferEntry& entry : mDynLogic->outputDestructionBuffer){
            switch(entry.type){
                case DynamicsWorldThreadLogic::ObjectDestructionType::DESTRUCTION_TYPE_BODY: {
                    btCollisionShape* s = entry.body->getCollisionShape();
                    if(mPendingShapes.find(s) != mPendingShapes.end()){
                        //delete s;
                        _destroyCollisionShape(s);
                        mPendingShapes.erase(s);
                    }
                    _destroyRigidBody(entry.body);
                    mPendingBodies.erase(entry.body);
                    break;
                }
                default:
                    break;
                //TODO there should also be destruction type chunk here as well.
            };
        }

        mDynLogic->outputDestructionBuffer.clear();
    }

    void PhysicsBodyDestructor::_destroyRigidBody(btRigidBody* bdy){
        DynamicsWorldMotionState* motionState = (DynamicsWorldMotionState*)bdy->getMotionState();
        if(motionState){
            delete motionState;
        }
        delete bdy;
    }

    void PhysicsBodyDestructor::_destroyCollisionShape(btCollisionShape* shape){
        if(shape->getShapeType() == TERRAIN_SHAPE_PROXYTYPE){
            //Heightfield shapes contain some extra data, so I need to be sure to deal with that correctly.
            btHeightfieldTerrainShape* terrShape = (btHeightfieldTerrainShape*)shape;

            //Call the code to delete the terrain here.
            //mTerrainManager->releaseTerrainDataPtr(shape->getUserPointer());
            BaseSingleton::getTerrainManager()->releaseTerrainDataPtr(shape->getUserPointer());
        }
        delete shape;
    }

    void PhysicsBodyDestructor::_destroyCollisionObject(btCollisionObject* object){
        //TODO I'll want to do things like release user data with this function.
        PhysicsMetaDataManager::releaseDataForObject(object->getUserIndex3());
        delete object;
    }

    bool destroyComplete[MAX_COLLISION_WORLDS + 1];
    bool clearedCollisionWorld = false;
    bool clearedEverything = false;
    void PhysicsBodyDestructor::_checkWorldDestructionReceipt(){
        //If the destructor is not waiting for the world to provide receipt of the fact that a destruction happened.
        if(!mWorldDestructionPending) return;

        //Perform the necessary checks.

        if(mDynLogic->checkWorldDestroyComplete()){

            //We only ever have one dynamics world, so the buffers can be cleared immediately if this is true. In the collision worlds we have to wait for them all to be done before clearing can take place.

            //Clear any leftover state before word shutdown.
            //As of right now I'm not clearing the regular input buffer, because I suppose things like setVelocity on a body won't matter. I might be wrong!

            //This is done here because there's a disconnect between world shutdown on the main thread, and when the physics thread realises this.
            //A race condition I had before was this:
            //World destruction on the main thread. _clearState is called so bodies are deleted.
            //The physics thread might take a hypothetical infinite time to clear these buffers and register the world destruction.
            //In the mean time on the main thread another delete request could have come in, where it would have been inserted into the destruction input buffer.
            //Now let's say the physics thread registers the shutdown, and clears the buffers, losing the most recent request.
            //Doing the clear here guarantees no race between the clearing of the buffers and requests coming in.
            {
                std::unique_lock<std::mutex> outputDestructionLock(mDynLogic->outputDestructionBufferMutex);
                mDynLogic->outputDestructionBuffer.clear();
            }
            {
                std::unique_lock<std::mutex> inputBufferLock(mDynLogic->objectInputBufferMutex);
                mDynLogic->inputObjectCommandBuffer.clear();
            }

            //The buffers are cleared first because then there's no chance of the thread getting to some stuff which is about to be deleted in _clearState.
            destroyComplete[0] = true;
        }
        for(uint8 i = 0; i < MAX_COLLISION_WORLDS; i++){
            CollisionWorldThreadLogic* targetLogic = mCollisionLogic[i];
            if(!targetLogic) continue;

            //Check if the destruction is complete.
            if(targetLogic->checkWorldDestroyComplete()){
                destroyComplete[i + 1] = true;
            }
        }

        //TODO find a way that includes all the worlds.
        if(destroyComplete[1] && !clearedCollisionWorld){
            {
                {
                    std::unique_lock<std::mutex> outputDestructionLock(mCollisionObjectDestructionBufferMutex);
                    mCollisonObjectDestructionBuffer.clear();
                }

                for(uint8 i = 0; i < MAX_COLLISION_WORLDS; i++){
                    CollisionWorldThreadLogic* targetLogic = mCollisionLogic[i];
                    if(!targetLogic) continue;

                    std::unique_lock<std::mutex> inputBufferLock(targetLogic->objectInputBufferMutex);
                    targetLogic->inputObjectCommandBuffer.clear();
                }
            }
            clearedCollisionWorld = true;
        }
        if(destroyComplete[1] && destroyComplete[0] && !clearedEverything){
            clearedEverything = true;

            //The world was destroyed on the thread successfully.
            _clearState();
            mWorldRecentlyDestroyed = true;
        }
    }

    //These setter functions are called by the main thread, so no checks necessary.
    void PhysicsBodyDestructor::setDynamicsWorldThreadLogic(DynamicsWorldThreadLogic* dynLogic){
        mDynLogic = dynLogic;
    }

    void PhysicsBodyDestructor::setCollisionWorldThreadLogic(uint8 worldId, CollisionWorldThreadLogic* collisionLogic){
        assert(worldId < MAX_COLLISION_WORLDS);
        mCollisionLogic[worldId] = collisionLogic;
    }

    bool PhysicsBodyDestructor::worldEventReceiver(const Event &e){
        const WorldEvent& event = (WorldEvent&)e;

        if(event.eventId() == EventId::WorldCreated){
            const WorldEventCreated& wEvent = (WorldEventCreated&)event;
            //The destruction might need to keep a reference to the dynamics world, so it can confirm with it that shapes were removed from the list.
            //However, I'm not sure this is necessary, so it's commented out until I've figured that out!
            //mDynWorld = wEvent->getPhysicsManager()->getDynamicsWorld();
        }
        else if(event.eventId() == EventId::WorldDestroyed){

            //mWorldRecentlyDestroyed = true;
            mWorldDestructionPending = true;
        }

        return false;
    }
}
