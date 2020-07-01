#include "CollisionWorld.h"

#include "Threading/Thread/Physics/CollisionWorldThreadLogic.h"
#include "World/Physics/PhysicsCollisionDataManager.h"

#ifdef DEBUGGING_TOOLS
    #include "World/WorldSingleton.h"
    #include "World/Developer/MeshVisualiser.h"
#endif

#include "Logger/Log.h"

namespace AV{
    ScriptDataPacker<PhysicsTypes::CollisionObjectEntry>* CollisionWorld::mCollisionObjectData;

    CollisionWorld::CollisionWorld(CollisionWorldId id, std::shared_ptr<CollisionWorldDataManager> dataManager)
        : mWorldId(id),
         mDataManager(dataManager) {

    }

    CollisionWorld::~CollisionWorld(){

    }

    void CollisionWorld::_resetBufferEntries(btCollisionObject* o){
        for(CollisionWorldThreadLogic::ObjectCommandBufferEntry& e : mThreadLogic->inputObjectCommandBuffer){
            if(e.type == CollisionWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_NONE
                || e.object != 0) continue;

            //Here we can assume a match was found.
            //I just invalidate the entry rather than deleting it, as it's going to be more efficient than shifting the vector.
            e.type = CollisionWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_NONE;
            e.object = 0;
        }
    }

    bool CollisionWorld::_objectInWorld(btCollisionObject* bdy) const{
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

    void CollisionWorld::notifyOriginShift(Ogre::Vector3 offset){

    }

    void CollisionWorld::setCollisionWorldThreadLogic(CollisionWorldThreadLogic* threadLogic){
        mThreadLogic = threadLogic;
        assert(mThreadLogic->getWorldId() == mWorldId);
    }

    void CollisionWorld::addObject(PhysicsTypes::CollisionObjectPtr object){
        if(!mThreadLogic) return;

        btCollisionObject* b = mCollisionObjectData->getEntry(object.get()).first;

        if(_objectInWorld(b)) return;
        mObjectsInWorld.insert(b);

        #ifdef DEBUGGING_TOOLS
            World* w = WorldSingleton::getWorld();
            assert(w);
            w->getMeshVisualiser()->insertCollisionObject(mWorldId, b);
        #endif

        std::unique_lock<std::mutex> inputBufferLock(mThreadLogic->objectInputBufferMutex);

        _resetBufferEntries(b);
        mThreadLogic->inputObjectCommandBuffer.push_back({CollisionWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_ADD_OBJECT, b});
    }

    void CollisionWorld::removeObject(PhysicsTypes::CollisionObjectPtr object){
        if(!mThreadLogic) return;

        btCollisionObject* b = mCollisionObjectData->getEntry(object.get()).first;

        if(!_objectInWorld(b)) return;
        mObjectsInWorld.erase(b);

        #ifdef DEBUGGING_TOOLS
            World* w = WorldSingleton::getWorld();
            assert(w);
            w->getMeshVisualiser()->removeCollisionObject(mWorldId, b);
        #endif

        std::unique_lock<std::mutex> inputBufferLock(mThreadLogic->objectInputBufferMutex);

        _resetBufferEntries(b);
        mThreadLogic->inputObjectCommandBuffer.push_back({CollisionWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_REMOVE_OBJECT, b});
    }

    void CollisionWorld::setObjectPosition(PhysicsTypes::CollisionObjectPtr object, const btVector3& pos){
        if(!mThreadLogic) return;
        //TODO for each of these functions I also need to check that the collision object is actually a part of this collision world.

        btCollisionObject* b = mCollisionObjectData->getEntry(object.get()).first;
        std::unique_lock<std::mutex> inputBufferLock(mThreadLogic->inputBufferMutex);

        mThreadLogic->inputCommandBuffer.push_back({CollisionWorldThreadLogic::InputBufferType::COMMAND_TYPE_SET_POSITION, b, pos});
    }

    void CollisionWorld::_removeObject(const btCollisionObject* object){
        //When this is sorted out, this will eventually be dependant on which world the object was created in.
        CollisionWorldId targetId = 0;

        #ifdef DEBUGGING_TOOLS
            World* w = WorldSingleton::getWorld();
            assert(w);
            w->getMeshVisualiser()->removeCollisionObject(targetId, object);
        #endif

        //TODO this needs finishing.
    }
}
