#pragma once

#include "Event.h"

namespace AV{
    class ChunkEvent : public Event{
    public:
        AV_EVENT_TYPE(EventType::Chunk)
        AV_EVENT_ID(EventId::Null)
    };

    class ChunkEventChunkEntered : public ChunkEvent{
    public:
        AV_EVENT_TYPE(EventType::Chunk)
        AV_EVENT_ID(EventId::ChunkEntered)

        int chunkX, chunkY;
    };

    class ChunkEventChunkLeft : public ChunkEvent{
    public:
        AV_EVENT_TYPE(EventType::Chunk)
        AV_EVENT_ID(EventId::ChunkLeft)

        int chunkX, chunkY;
    };
}
