#pragma once

#include "Event.h"
#include "OgreString.h"

namespace AV{
    class SystemEvent : public Event{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::Null)
    };

    class SystemEventEngineClose : public SystemEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::SystemEngineClose)
    };

    class SystemEventWindowResize : public SystemEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::SystemWindowResize)

        int width, height;
    };

    class SystemEventInputDeviceAdded : public SystemEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::SystemInputDeviceAdded)
    };

    class SystemEventInputDeviceRemoved : public SystemEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::SystemInputDeviceRemoved)
    };
}
