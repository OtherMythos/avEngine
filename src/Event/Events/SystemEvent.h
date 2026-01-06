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

    class SystemEventWindowMinimized : public SystemEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::SystemWindowMinimized)
    };
    class SystemEventWindowMaximised : public SystemEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::SystemWindowMaximised)
    };
    class SystemEventWindowFocusGained : public SystemEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::SystemWindowFocusGained)
    };
    class SystemEventWindowFocusLost : public SystemEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::SystemWindowFocusLost)
    };
    class SystemEventWindowShown : public SystemEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::SystemWindowShown)
    };
    class SystemEventWindowHidden : public SystemEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::SystemWindowHidden)
    };
    class SystemEventWindowRestored : public SystemEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::SystemWindowRestored)
    };
    class SystemEventWindowExposed : public SystemEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::SystemWindowExposed)
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

    class SystemEventInputTouch : public SystemEvent{
    public:
        uint64 fingerId;
    };
    class SystemEventInputTouchBegan : public SystemEventInputTouch{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::SystemInputTouchBegan)

        //If a gui element was intersected (pressed) by this touch, this will be true.
        bool guiIntersected = false;
    };
    class SystemEventInputTouchEnded : public SystemEventInputTouch{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::SystemInputTouchEnded)
    };
    class SystemEventInputTouchMotion : public SystemEventInputTouch{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::SystemInputTouchMotion)
    };

    class SystemEventApplicationTerminating : public SystemEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::SystemApplicationTerminating)
    };

    class SystemEventApplicationLowMemory : public SystemEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::SystemApplicationLowMemory)
    };

    class SystemEventApplicationWillEnterBackground : public SystemEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::SystemApplicationWillEnterBackground)
    };

    class SystemEventApplicationDidEnterBackground : public SystemEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::SystemApplicationDidEnterBackground)
    };

    class SystemEventApplicationWillEnterForeground : public SystemEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::SystemApplicationWillEnterForeground)
    };

    class SystemEventApplicationDidEnterForeground : public SystemEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::SystemApplicationDidEnterForeground)
    };
}
