#pragma once

namespace AV{
    #define AV_EVENT_TYPE(eType) virtual EventType type() const { return eType; }
    #define AV_EVENT_ID(eType) virtual EventId eventId() const { return eType; }

    enum class EventId{
        Null = 0,

        //World events
        WorldMapChange,
        WorldOriginChange,
        WorldPlayerRadiusChange,
        WorldPlayerPositionChange,
        WorldCreated,
        WorldDestroyed,
        WorldBecameReady,
        WorldBecameUnready,

        //Chunk events
        ChunkEntered,
        ChunkLeft,

        //System event
        SystemEngineClose,
        SystemWindowResize,

        #ifdef TEST_MODE
            //Testing events
            TestingBooleanAssertFailed,
            TestingComparisonAssertFailed,
            TestingTestEnd,
            TestingScriptFailure,
            TestingTimeoutReached,
        #endif

        EVENT_ID_END,
    };

    static const char* const EventIdStr[] = {
        "_EVENT_NULL",

        //World
        "_EVENT_WORLD_MAP_CHANGE",
        "_EVENT_WORLD_ORIGIN_CHANGE",
        "_EVENT_WORLD_PLAYER_RADIUS_CHANGE",
        "_EVENT_WORLD_PLAYER_POSITION_CHANGE",
        "_EVENT_WORLD_CREATED",
        "_EVENT_WORLD_DESTROYED",
        "_EVENT_WORLD_BECAME_READY",
        "_EVENT_WORLD_BECAME_UNREADY",

        //Chunk
        "_EVENT_CHUNK_ENTERED",
        "_EVENT_CHUNK_LEFT",

        //System
        "_EVENT_SYSTEM_ENGINE_CLOSE",
        "_EVENT_SYSTEM_WINDOW_RESIZE",

        //Testing
        #ifdef TEST_MODE
            //Really these aren't used in scripts, but I have them there for continuity.
            "_EVENT_TESTING_BOOLEAN_ASSERT_FAILED",
            "_EVENT_TESTING_COMPARISON_ASSERT_FAILED",
            "_EVENT_TESTING_TEST_END",
            "_EVENT_TESTING_SCRIPT_FAILURE",
            "_EVENT_TESTING_TIMEOUT_REACHED",
        #endif
    };
    static_assert(sizeof(EventIdStr) / sizeof(const char*) == static_cast<int>(EventId::EVENT_ID_END));

    enum class EventType{
        Null = 0,
        World,
        System,
        #ifdef TEST_MODE
            Testing,
        #endif
        Chunk
    };

    class Event{
    public:
        AV_EVENT_TYPE(EventType::Null)
        AV_EVENT_ID(EventId::Null)
    };
};
