#include "CollisionWorld.h"

#include "Threading/Thread/Physics/CollisionWorldThreadLogic.h"
#include "World/Physics/PhysicsCollisionDataManager.h"

#ifdef DEBUGGING_TOOLS
    #include "World/WorldSingleton.h"
    #include "World/Developer/MeshVisualiser.h"
#endif

#include "Logger/Log.h"

namespace AV{
    DataPacker<PhysicsTypes::CollisionObjectEntry>* CollisionWorld::mCollisionObjectData;

    //A list of the created worlds. This reference is necessary for shared pointer deletion of objects.
    CollisionWorld* staticCollisionWorlds[MAX_COLLISION_WORLDS];

    CollisionWorld::CollisionWorld(CollisionWorldId id)
        : mWorldId(id){

        assert(!staticCollisionWorlds[mWorldId]);
        staticCollisionWorlds[mWorldId] = this;
    }

    CollisionWorld::~CollisionWorld(){
        staticCollisionWorlds[mWorldId] = 0;
    }

    void CollisionWorld::_resetBufferEntries(const btCollisionObject* o){
        for(CollisionWorldThreadLogic::ObjectCommandBufferEntry& e : mThreadLogic->inputObjectCommandBuffer){
            if(e.type == CollisionWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_NONE
                || e.object != o) continue;

            //Here we can assume a match was found.
            //I just invalidate the entry rather than deleting it, as it's going to be more efficient than shifting the vector.
            e.type = CollisionWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_NONE;
            e.object = 0;
        }
    }

    bool CollisionWorld::_objectInWorld(const btCollisionObject* bdy) const{
        return mObjectsInWorld.find(bdy) != mObjectsInWorld.end();
    }

    void CollisionWorld::update(){
        mCollisionEvents.clear();
        {
            std::unique_lock<std::mutex> outputBufferLock(mThreadLogic->objectOutputBufferMutex);

            //Write these values into an intermediate place to be processed later.
            //This needs to happen as I need to call scripts and other stuff with this data.
            //This might take a while, so copying the data is worth doing.
            for(const CollisionWorldThreadLogic::ObjectEventBufferEntry& e : mThreadLogic->outputObjectEventBuffer){
                mCollisionEvents.push_back({e.sender, e.receiver, e.eventType});
            }

            mThreadLogic->outputObjectEventBuffer.clear();
        }

        for(const CollisionEventEntry& e : mCollisionEvents){
            PhysicsCollisionDataManager::processCollision(e.sender, e.receiver, e.eventMask);
        }

    }

    void CollisionWorld::notifyOriginShift(const Ogre::Vector3 &offset, const SlotPosition& newPos){
        if(!mThreadLogic) return;
        std::unique_lock<std::mutex> inputBufferLock(mThreadLogic->objectInputBufferMutex);

        btVector3 orig(offset.x, offset.y, offset.z);
        mThreadLogic->worldOriginChangeOffset = orig;
        mThreadLogic->worldOriginChangeNewPosition = newPos;
        mThreadLogic->worldShifted = true;

        mShiftPerformedLastFrame = true;
    }

    void CollisionWorld::setCollisionWorldThreadLogic(CollisionWorldThreadLogic* threadLogic){
        mThreadLogic = threadLogic;
        assert(mThreadLogic->getWorldId() == mWorldId);
    }

    CollisionWorld::CollisionFunctionStatus CollisionWorld::addObject(PhysicsTypes::CollisionObjectPtr object){
        if(!mThreadLogic) return NO_WORLD;

        btCollisionObject* b = mCollisionObjectData->getEntry(object.get()).first;
        uint8 worldId = CollisionWorldUtils::_readPackedIntWorldId(b->getUserIndex());
        if(worldId != mWorldId) return WRONG_WORLD;

        if(_objectInWorld(b)) return ALREADY_IN_WORLD;
        mObjectsInWorld.insert(b);

        #ifdef DEBUGGING_TOOLS
            World* w = WorldSingleton::getWorld();
            assert(w);
            w->getMeshVisualiser()->insertCollisionObject(mWorldId, b);
        #endif

        std::unique_lock<std::mutex> inputBufferLock(mThreadLogic->objectInputBufferMutex);

        _resetBufferEntries(b);
        mThreadLogic->inputObjectCommandBuffer.push_back({CollisionWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_ADD_OBJECT, b, 0, 0});

        return SUCCESS;
    }

    CollisionWorld::CollisionFunctionStatus CollisionWorld::removeObject(PhysicsTypes::CollisionObjectPtr object){
        if(!mThreadLogic) return NO_WORLD;

        btCollisionObject* b = mCollisionObjectData->getEntry(object.get()).first;
        if(CollisionWorldUtils::_readPackedIntWorldId(b->getUserIndex()) != mWorldId) return WRONG_WORLD;

        if(!_objectInWorld(b)) return NOT_IN_WORLD;
        mObjectsInWorld.erase(b);

        #ifdef DEBUGGING_TOOLS
            World* w = WorldSingleton::getWorld();
            assert(w);
            w->getMeshVisualiser()->removeCollisionObject(mWorldId, b);
        #endif

        std::unique_lock<std::mutex> inputBufferLock(mThreadLogic->objectInputBufferMutex);

        _resetBufferEntries(b);
        mThreadLogic->inputObjectCommandBuffer.push_back({CollisionWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_REMOVE_OBJECT, b, 0, 0});

        return SUCCESS;
    }

    CollisionWorld::CollisionFunctionStatus CollisionWorld::_setPositionInternal(btCollisionObject* b, const btVector3& pos){
        if(!mThreadLogic) return NO_WORLD;

        #ifdef DEBUGGING_TOOLS
            World* w = WorldSingleton::getWorld();
            assert(w);
            w->getMeshVisualiser()->setCollisionObjectPosition(Ogre::Vector3(pos), b);
        #endif

        std::unique_lock<std::mutex> inputBufferLock(mThreadLogic->inputBufferMutex);

        mThreadLogic->inputCommandBuffer.push_back({CollisionWorldThreadLogic::InputBufferType::COMMAND_TYPE_SET_POSITION, b, pos});

        return SUCCESS;
    }

    void CollisionWorld::setObjectPositionStatic(PhysicsTypes::CollisionObjectPtr object, const btVector3& pos){
        btCollisionObject* b = mCollisionObjectData->getEntry(object.get()).first;
        uint8 worldId = CollisionWorldUtils::_readPackedIntWorldId(b->getUserIndex());
        assert(worldId < MAX_COLLISION_WORLDS);

        staticCollisionWorlds[worldId]->_setPositionInternal(b, pos);

    }

    CollisionWorld::CollisionFunctionStatus CollisionWorld::setObjectPosition(PhysicsTypes::CollisionObjectPtr object, const btVector3& pos){
        btCollisionObject* b = mCollisionObjectData->getEntry(object.get()).first;
        if(CollisionWorldUtils::_readPackedIntWorldId(b->getUserIndex()) != mWorldId) return WRONG_WORLD;

        return _setPositionInternal(b, pos);
    }

    uint8 CollisionWorld::getObjectWorld(PhysicsTypes::CollisionObjectPtr object){
        assert(mCollisionObjectData);
        btCollisionObject* b = mCollisionObjectData->getEntry(object.get()).first;
        return CollisionWorldUtils::_readPackedIntWorldId(b->getUserIndex());
    }

    CollisionObjectType::CollisionObjectType CollisionWorld::getObjectType(PhysicsTypes::CollisionObjectPtr object){
        assert(mCollisionObjectData);
        btCollisionObject* b = mCollisionObjectData->getEntry(object.get()).first;
        return CollisionWorldUtils::_readPackedIntType(b->getUserIndex());
    }

    //Static function
    uint32 CollisionWorld::addCollisionObjectChunk(const PhysicsTypes::CollisionChunkEntry& chunk){
        for(uint8 i = 0; i < MAX_COLLISION_WORLDS; i++){
            CollisionWorld* targetWorld = staticCollisionWorlds[i];
            if(!targetWorld) continue;

            std::unique_lock<std::mutex> inputBufferLock(targetWorld->mThreadLogic->objectInputBufferMutex);

            //PhysicsTypes::CollisionObjectsVector b = chunk.second;
            btRigidBody* vectorObjectEntries = reinterpret_cast<btRigidBody*>(chunk.second);
            targetWorld->_resetBufferEntries(vectorObjectEntries);
            targetWorld->mThreadLogic->inputObjectCommandBuffer.push_back({CollisionWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_ADD_CHUNK, vectorObjectEntries, chunk.slotX, chunk.slotY});
        }

        return 0;
    }

    //Static function, called during shared pointer destruction.
    void CollisionWorld::_removeObject(const btCollisionObject* object){
        //When this is sorted out, this will eventually be dependant on which world the object was created in.
        CollisionWorldId targetId = CollisionWorldUtils::_readPackedIntWorldId(object->getUserIndex());

        #ifdef DEBUGGING_TOOLS
            World* w = WorldSingleton::getWorld();
            //If this is not true, likely the world is shutting down
            if(w){
                w->getMeshVisualiser()->removeCollisionObject(targetId, object);
            }
        #endif

        CollisionWorld* targetWorld = staticCollisionWorlds[targetId];
        if(!targetWorld){
            //This can happen if the world does not exist. In this case we have nothing to do.
            return;
        }

        //The buffer still needs to be cleared.
        {
            std::unique_lock<std::mutex> inputBufferLock(targetWorld->mThreadLogic->objectInputBufferMutex);
            targetWorld->_resetBufferEntries(object);
        }

        targetWorld->mObjectsInWorld.erase(object);
    }
}
