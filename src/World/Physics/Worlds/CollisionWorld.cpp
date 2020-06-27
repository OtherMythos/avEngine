#include "CollisionWorld.h"

#include "Threading/Thread/Physics/CollisionWorldThreadLogic.h"
#include "World/Physics/PhysicsCollisionDataManager.h"

#include "Logger/Log.h"

namespace AV{
    ScriptDataPacker<PhysicsTypes::CollisionObjectEntry>* CollisionWorld::mCollisionObjectData;

    CollisionWorld::CollisionWorld(CollisionWorldId id, std::shared_ptr<CollisionWorldDataManager> dataManager)
        : mWorldId(id),
         mDataManager(dataManager) {

    }

    CollisionWorld::~CollisionWorld(){

    }

    void CollisionWorld::update(){
        mCollisionEvents.clear();
        {
            std::unique_lock<std::mutex> outputBufferLock(mThreadLogic->objectOutputBufferMutex);

            //Write these values into an intermediate place to be processed later.
            //This needs to happen as I need to call scripts and other stuff with this data.
            //This might take a while, so copying the data is worth doing.
            for(const CollisionWorldThreadLogic::ObjectEventBufferEntry& e : mThreadLogic->outputObjectEventBuffer){
                mCollisionEvents.push_back({e.first, e.second, e.eventType});
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

        //<Check if the body is in the world here>

        std::unique_lock<std::mutex> inputBufferLock(mThreadLogic->objectInputBufferMutex);

        //<Reset the current buffer entries>
        mThreadLogic->inputObjectCommandBuffer.push_back({CollisionWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_ADD_OBJECT, b});

        // btRigidBody* b = mBodyData->getEntry(body.get()).first;
        // if(_bodyInWorld(b)) return;
        //
        // mBodiesInWorld.insert(b);
        //
        // std::unique_lock<std::mutex> inputBufferLock(mThreadLogic->objectInputBufferMutex);
        //
        //
        // //Do a search for any entries in the buffer with the same pointer and invalidate them.
        // _resetBufferEntries(b);
        // mThreadLogic->inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_ADD_BODY, b});
    }

    void CollisionWorld::removeObject(PhysicsTypes::CollisionObjectPtr object){
        if(!mThreadLogic) return;

        btCollisionObject* b = mCollisionObjectData->getEntry(object.get()).first;
        std::unique_lock<std::mutex> inputBufferLock(mThreadLogic->objectInputBufferMutex);

        mThreadLogic->inputObjectCommandBuffer.push_back({CollisionWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_REMOVE_OBJECT, b});
    }

    void CollisionWorld::setObjectPosition(PhysicsTypes::CollisionObjectPtr object, const btVector3& pos){
        if(!mThreadLogic) return;
        //TODO for each of these functions I also need to check that the collision object is actually a part of this collision world.

        btCollisionObject* b = mCollisionObjectData->getEntry(object.get()).first;
        std::unique_lock<std::mutex> inputBufferLock(mThreadLogic->inputBufferMutex);

        mThreadLogic->inputCommandBuffer.push_back({CollisionWorldThreadLogic::InputBufferType::COMMAND_TYPE_SET_POSITION, b, pos});
    }
}
