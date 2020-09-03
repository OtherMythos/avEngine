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
    };

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
