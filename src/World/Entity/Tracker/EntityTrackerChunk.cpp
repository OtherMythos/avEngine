#include "EntityTrackerChunk.h"

#include "World/Entity/EntityManager.h"

namespace AV{
    EntityTrackerChunk::EntityTrackerChunk(){

    }

    EntityTrackerChunk::~EntityTrackerChunk(){
        
    }
    
    void EntityTrackerChunk::addEntity(eId e){
        mEntities.insert(e);
    }
    
    bool EntityTrackerChunk::removeEntity(eId e){
        auto it = mEntities.find(e);
        if(it != mEntities.end()){
            mEntities.erase(it);
            return true;
        }else return false;
    }
    
    bool EntityTrackerChunk::containsEntity(eId e){
        return mEntities.count(e) > 0;
    }
    
    void EntityTrackerChunk::destroyChunk(EntityManager* entityManager){
        for(eId e : mEntities){
            entityManager->destroyKnownEntity(e, false);
        }
        
        mEntities.clear();
    }

}
