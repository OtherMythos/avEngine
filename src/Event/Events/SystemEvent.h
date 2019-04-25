#pragma once

#include "Event.h"
#include "OgreString.h"

namespace AV{

    #define AV_EVENT_CATEGORY(c) virtual SystemEventCategory eventCategory() const { return c; };

    enum class SystemEventCategory{
        Null,
        EngineClose,
        WindowResize
    };

    class SystemEvent : public Event{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_CATEGORY(SystemEventCategory::Null)
    };

    class SystemEventEngineClose : public SystemEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_CATEGORY(SystemEventCategory::EngineClose)
    };

    class SystemEventWindowResize : public SystemEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_CATEGORY(SystemEventCategory::WindowResize)

        int width, height;
    };
}
