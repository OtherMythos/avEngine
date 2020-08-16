#pragma once

#include "Event.h"

namespace AV{
    #define AV_CHUNK_EVENT_CATEGORY(c) virtual ChunkEventCategory eventCategory() const { return c; };

    enum class ChunkEventCategory{
        Null,
        ChunkEntered,
        ChunkLeft
    };

    class ChunkEvent : public Event{
    public:
        AV_EVENT_TYPE(EventType::Chunk)
        AV_CHUNK_EVENT_CATEGORY(ChunkEventCategory::Null)
    };

    class ChunkEventChunkEntered : public ChunkEvent{
    public:
        AV_EVENT_TYPE(EventType::Chunk)
        AV_CHUNK_EVENT_CATEGORY(ChunkEventCategory::ChunkEntered)

        int chunkX, chunkY;
    };

    class ChunkEventChunkLeft : public ChunkEvent{
    public:
        AV_EVENT_TYPE(EventType::Chunk)
        AV_CHUNK_EVENT_CATEGORY(ChunkEventCategory::ChunkLeft)

        int chunkX, chunkY;
    };
}
