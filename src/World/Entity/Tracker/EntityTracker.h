#pragma once

#include "World/Slot/SlotPosition.h"
#include "World/Entity/eId.h"

namespace AV {
    class Event;
    class EntityTrackerChunk;
    class EntityManager;
    class ImguiEntityView;

    class EntityTracker{
        friend ImguiEntityView;
    public:
        EntityTracker();
        ~EntityTracker();
        
        struct TrackingChunkInfo{
            int chunkX, chunkY;
            int trackedEntities;
        };

        /**
        Track a 'known' entity.
        This function is intended for use when the EntityManager creates entities, as the necessary information (position, tracking status) is already known.
         It does less sanity checks on the input, such as confirming the target destination for the entity is actually trackable.

        @remarks
        This function assumes the entity is not currently tracked.
        */
        bool trackKnownEntity(eId e, SlotPosition pos);
        /**
         Set an entity to be tracked.
         unlike the trackKnownEntity function, this will perform sanity checks.
         */
        bool trackEntity(eId e);
        bool untrackEntity(eId e);

        bool worldEventReceiver(const Event &e);
        bool updateEntity(eId e, SlotPosition oldPos, SlotPosition newPos, EntityManager *entityManager);

        int getTrackedEntities() { return mTrackedEntities; }

    private:
        typedef std::pair<int, int> ChunkEntry;
        std::map<ChunkEntry, EntityTrackerChunk*> mEChunks;

        bool _eChunkExists(ChunkEntry e);
        int mTrackedEntities = 0;
    };
}
