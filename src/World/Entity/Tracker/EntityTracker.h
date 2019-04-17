#pragma once

#include "World/Slot/SlotPosition.h"
#include "World/Entity/eId.h"

namespace AV {
    class Event;
    class EntityTrackerChunk;
    
    class EntityTracker{
    public:
        EntityTracker();
        ~EntityTracker();
        
        bool trackEntity(eId e, SlotPosition pos);
        
        bool worldEventReceiver(const Event &e);
        
    private:
        typedef std::pair<int, int> ChunkEntry;
        std::map<ChunkEntry, EntityTrackerChunk*> mEChunks;
        
        bool _eChunkExists(ChunkEntry e);
    };
}
