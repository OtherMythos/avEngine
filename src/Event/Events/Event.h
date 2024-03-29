#pragma once

#include "System/EnginePrerequisites.h"

namespace AV{
    #define AV_EVENT_TYPE(eType) virtual EventType type() const { return eType; }
    #define AV_EVENT_ID(eType) virtual EventId eventId() const { return eType; }

    enum class EventId : uint32{
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
        SystemWindowMinimized,
        SystemWindowMaximised,
        SystemWindowFocusGained,
        SystemWindowFocusLost,
        SystemWindowShown,
        SystemWindowHidden,
        SystemWindowRestored,
        SystemWindowExposed,

        SystemInputDeviceAdded,
        SystemInputDeviceRemoved,
        SystemInputTouchBegan,
        SystemInputTouchEnded,
        SystemInputTouchMotion,

        #ifdef TEST_MODE
            //Testing events
            TestingBooleanAssertFailed,
            TestingComparisonAssertFailed,
            TestingTestEnd,
            TestingScriptFailure,
            TestingTimeoutReached,
        #endif

        #ifdef DEBUGGING_TOOLS
            DebuggingToolToggle,
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
        "_EVENT_SYSTEM_WINDOW_MINIMIZED",
        "_EVENT_SYSTEM_WINDOW_MAXIMISED",
        "_EVENT_SYSTEM_WINDOW_FOCUS_GAINED",
        "_EVENT_SYSTEM_WINDOW_FOCUS_LOST",
        "_EVENT_SYSTEM_WINDOW_SHOWN",
        "_EVENT_SYSTEM_WINDOW_HIDDEN",
        "_EVENT_SYSTEM_WINDOW_RESTORED",
        "_EVENT_SYSTEM_WINDOW_EXPOSED",

        "_EVENT_SYSTEM_INPUT_DEVICE_ADDED",
        "_EVENT_SYSTEM_INPUT_DEVICE_REMOVED",
        "_EVENT_SYSTEM_INPUT_TOUCH_BEGAN",
        "_EVENT_SYSTEM_INPUT_TOUCH_ENDED",
        "_EVENT_SYSTEM_INPUT_TOUCH_MOTION",

        //Testing
        #ifdef TEST_MODE
            //Really these aren't used in scripts, but I have them there for continuity.
            "_EVENT_TESTING_BOOLEAN_ASSERT_FAILED",
            "_EVENT_TESTING_COMPARISON_ASSERT_FAILED",
            "_EVENT_TESTING_TEST_END",
            "_EVENT_TESTING_SCRIPT_FAILURE",
            "_EVENT_TESTING_TIMEOUT_REACHED",
        #endif

        #ifdef DEBUGGING_TOOLS
            "_DEBUGGING_TOOL_TOGGLE",
        #endif
    };
    #ifndef _WIN32
        //Probably temporary until I switch to c++ 17.
        static_assert(sizeof(EventIdStr) / sizeof(const char*) == static_cast<int>(EventId::EVENT_ID_END), "Mismatch ids to labels");
    #endif

    enum class EventType{
        Null = 0,
        World,
        System,
        #ifdef TEST_MODE
            Testing,
        #endif
        #ifdef DEBUGGING_TOOLS
            DebuggerTools,
        #endif
        Chunk
    };

    class Event{
    public:
        AV_EVENT_TYPE(EventType::Null)
        AV_EVENT_ID(EventId::Null)
    };
};
