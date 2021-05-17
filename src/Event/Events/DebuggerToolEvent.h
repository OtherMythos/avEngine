#pragma once

#include "Event.h"

namespace AV{
    class DebuggerToolEvent : public Event{
    public:
        AV_EVENT_TYPE(EventType::DebuggerTools)
        AV_EVENT_ID(EventId::Null)
    };

    enum class DebuggerToolToggle{
        StatsToggle,
        MeshesToggle
    };
    class DebuggerToolEventToggle : public DebuggerToolEvent{
    public:
        AV_EVENT_TYPE(EventType::DebuggerTools)
        AV_EVENT_ID(EventId::DebuggingToolToggle);

        DebuggerToolToggle t;
    };
}
