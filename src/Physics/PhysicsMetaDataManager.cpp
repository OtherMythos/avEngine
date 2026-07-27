#include "PhysicsMetaDataManager.h"

#include <stack>

namespace AV{

    CollisionInternalId mCollisionIdCounter;
    std::stack<CollisionInternalId> mFreeCollisionIds;
    DataPool<PhysicsMetaDataManager::PhysicsObjectMeta> PhysicsMetaDataManager::mDataStore;

    void PhysicsMetaDataManager::setup(){
        mCollisionIdCounter = 0;
    }

    void PhysicsMetaDataManager::shutdown(){
        while(!mFreeCollisionIds.empty()) mFreeCollisionIds.pop();
    }

    int PhysicsMetaDataManager::createDataForObject(){
        CollisionInternalId id = _getCollisionObjectInternalId();

        uint32 value = mDataStore.storeEntry({id, eId::INVALID});
        return static_cast<int>(value);
    }

    void PhysicsMetaDataManager::releaseDataForObject(int e){
        uint32 value = static_cast<uint32>(e);
        _releaseCollisionObjectInternalId(mDataStore.getEntry(value).id);
        mDataStore.removeEntry(value);
    }

    void PhysicsMetaDataManager::setEntityForObject(int id, eId e){
        uint32 value = static_cast<uint32>(id);
        mDataStore.getEntry(value).attachedEntity = e;
    }

    PhysicsMetaDataManager::PhysicsObjectMeta PhysicsMetaDataManager::getObjectMeta(int id){
        uint32 value = static_cast<uint32>(id);
        return mDataStore.getEntry(value);
    }

    CollisionInternalId PhysicsMetaDataManager::_getCollisionObjectInternalId(){
        if(mFreeCollisionIds.empty()) return mCollisionIdCounter++;
        CollisionInternalId retVal = mFreeCollisionIds.top();
        mFreeCollisionIds.pop();
        return retVal;
    }

    void PhysicsMetaDataManager::_releaseCollisionObjectInternalId(CollisionInternalId id){
        mFreeCollisionIds.push(id);
    }

}
