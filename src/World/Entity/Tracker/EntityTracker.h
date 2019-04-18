#pragma once

#include "World/Slot/SlotPosition.h"
#include "World/Entity/eId.h"

namespace AV {
    class Event;
    class EntityTrackerChunk;
    class EntityManager;

    class EntityTracker{
    public:
        EntityTracker();
        ~EntityTracker();

        /**
        Track a 'known' entity.
        This function is intended for use when the EntityManager creates entities, as the necessary information (position, tracking status) is already known.

        @remarks
        This function assumes the entity is not currently tracked.
        */
        bool trackKnownEntity(eId e, SlotPosition pos);
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
