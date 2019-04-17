#include "EntityTracker.h"

#include "World/Slot/SlotPosition.h"
#include "World/WorldSingleton.h"

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

    bool EntityTracker::trackEntity(eId e, SlotPosition pos){
        bool viableChunk = WorldSingleton::getWorld()->getChunkRadiusLoader()->chunkLoadedInCurrentMap(pos.chunkX(), pos.chunkY());
        //The chunk is not loaded (viable), so the entity cannot be tracked.
        if(!viableChunk) return false;
        
        //The entity can be tracked.
        ChunkEntry entry = ChunkEntry(pos.chunkX(), pos.chunkY());
        if(_eChunkExists(entry)){
            //Insert the entity into that echunk.
            mEChunks[entry]->addEntity(e);
        }else{
            //Create the chunk and insert it into the map.
            EntityTrackerChunk* c = new EntityTrackerChunk();
            mEChunks[entry] = c;
            c->addEntity(e);
        }


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
