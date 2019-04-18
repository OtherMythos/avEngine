#include "EntityTracker.h"

#include "World/Slot/SlotPosition.h"
#include "World/WorldSingleton.h"
#include "World/Entity/EntityManager.h"
#include "World/Entity/Logic/FundamentalLogic.h"

#include "Event/EventDispatcher.h"
#include "Event/Events/WorldEvent.h"
#include "World/Slot/ChunkRadiusLoader.h"

#include "World/Entity/Tracker/EntityTrackerChunk.h"

namespace AV {
    EntityTracker::EntityTracker(){
        EventDispatcher::subscribe(EventType::World, AV_BIND(EntityTracker::worldEventReceiver));
    }

    EntityTracker::~EntityTracker(){

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

    bool EntityTracker::updateEntity(eId e, SlotPosition oldPos, SlotPosition newPos, EntityManager *entityManager){

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
                entityManager->destroyEntity(e);
                mTrackedEntities--;
                return false;
            }
        }else newChunk = mEChunks[newChunkEntry];

        //Now we have both chunks, remove the entity from the old one and put it into the new one.
        oldChunk->removeEntity(e);
        newChunk->addEntity(e);

        return true;
    }

    bool EntityTracker::_eChunkExists(ChunkEntry e){
        if(mEChunks.find(e) != mEChunks.end()) return true;
        return false;
    }

    bool EntityTracker::worldEventReceiver(const Event &e){
        const WorldEvent& event = (WorldEvent&)e;

        return true;
    }
}
