#include "EntityTracker.h"

#include "World/Slot/SlotPosition.h"
#include "World/WorldSingleton.h"
#include "World/Entity/EntityManager.h"
#include "World/Entity/Logic/FundamentalLogic.h"

#include "Event/EventDispatcher.h"
#include "Event/Events/ChunkEvent.h"
#include "World/Slot/ChunkRadiusLoader.h"

#include "World/Entity/Tracker/EntityTrackerChunk.h"

namespace AV {
    EntityTracker::EntityTracker(){

    }

    EntityTracker::~EntityTracker(){

    }

    void EntityTracker::initialise(EntityManager* entityManager){
        mEntityManager = entityManager;
        EventDispatcher::subscribe(EventType::Chunk, AV_BIND(EntityTracker::chunkEventReceiver));
    }

    bool EntityTracker::trackKnownEntity(eId e, SlotPosition pos){
        ChunkEntry entry = ChunkEntry(pos.chunkX(), pos.chunkY());
        EntityTrackerChunk* c;
        if(_eChunkExists(entry)){
            c = mEChunks[entry];
        }else{
            //Create the chunk and insert it into the map.
            c = new EntityTrackerChunk();
            mEChunks[entry] = c;
        }
        //Insert the entity into that echunk.
        c->addEntity(e);
        mTrackedEntities++;

        return true;
    }

    bool EntityTracker::trackEntity(eId e){
        if(FundamentalLogic::getTracked(e)) return false;

        SlotPosition pos = FundamentalLogic::getPosition(e);

        //If this entity is not within the bounds of the player radius it should be deleted immediately.
        bool viableChunk = WorldSingleton::getWorld()->getChunkRadiusLoader()->chunkLoadedInCurrentMap(pos.chunkX(), pos.chunkY());
        if(!viableChunk){
            WorldSingleton::getWorld()->getEntityManager()->destroyEntity(e);
            return false;
        }
        //This set tracked is not supposed to be done in trackKnownEntity, as it's assumed that's already been set to true.
        FundamentalLogic::setTracked(e, true);
        return trackKnownEntity(e, pos);
    }

    bool EntityTracker::untrackEntity(eId e){
        if(!FundamentalLogic::getTracked(e)) return false;

        SlotPosition pos = FundamentalLogic::getPosition(e);

        ChunkEntry entry = ChunkEntry(pos.chunkX(), pos.chunkY());
        if(_eChunkExists(entry)){
            mEChunks[entry]->removeEntity(e);
        }else return false; //The entity is not tracked, as the chunk it resides in does not exist.

        FundamentalLogic::setTracked(e, false);
        mTrackedEntities--;
        return true;
    }

    bool EntityTracker::updateEntity(eId e, SlotPosition oldPos, SlotPosition newPos){

        ChunkEntry oldChunkEntry(oldPos.chunkX(), oldPos.chunkY());
        ChunkEntry newChunkEntry(newPos.chunkX(), newPos.chunkY());

        if(oldChunkEntry == newChunkEntry) return true;

        //If the old chunk doesn't exist then there's an error in the input.
        if(!_eChunkExists(oldChunkEntry)) return true;

        EntityTrackerChunk* oldChunk = mEChunks[oldChunkEntry];
        EntityTrackerChunk* newChunk = 0;

        if(!_eChunkExists(newChunkEntry)){
            //If the new chunk doesn't exist, we should first check if it is within the radius of the player.
            //If it is it can be created.
            bool viableChunk = WorldSingleton::getWorld()->getChunkRadiusLoader()->chunkLoadedInCurrentMap(newPos.chunkX(), newPos.chunkY());
            if(viableChunk){
                //The chunk is viable, so it can be created.
                newChunk = new EntityTrackerChunk();
                mEChunks[newChunkEntry] = newChunk;
            }else {
                //In this case the entity has walked into a chunk which is not valid, and should be removed.
                oldChunk->removeEntity(e);
                mEntityManager->destroyEntity(e);
                mTrackedEntities--;
                return false;
            }
        }else newChunk = mEChunks[newChunkEntry];

        //Now we have both chunks, remove the entity from the old one and put it into the new one.
        oldChunk->removeEntity(e);
        newChunk->addEntity(e);

        return true;
    }

    void EntityTracker::_destroyEChunk(ChunkEntry entry){
        if(!_eChunkExists(entry)) return;

        mTrackedEntities -= mEChunks[entry]->getEntityCount();

        mEChunks[entry]->destroyChunk(mEntityManager);
    }

    void EntityTracker::destroyTrackedEntities(){
        for(auto it = mEChunks.begin(); it != mEChunks.end(); it++){
            _destroyEChunk((*it).first);
        }
    }

    bool EntityTracker::_eChunkExists(ChunkEntry e){
        if(mEChunks.find(e) != mEChunks.end()) return true;
        return false;
    }

    bool EntityTracker::chunkEventReceiver(const Event &e){
        const ChunkEvent& event = (ChunkEvent&)e;
        if(event.eventCategory() == ChunkEventCategory::ChunkEntered){

        }else if(event.eventCategory() == ChunkEventCategory::ChunkLeft){
            //Unload the contents of that chunk.
            const ChunkEventChunkLeft& left = (ChunkEventChunkLeft&)event;

            ChunkEntry entry(left.chunkX, left.chunkY);
            _destroyEChunk(entry);
        }
        return true;
    }
}
