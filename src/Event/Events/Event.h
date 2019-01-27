#pragma once

namespace AV{
    enum class EventType{
        Null = 0,
        world
    };

    class Event{
    public:
        EventType type = EventType::Null;
    };
};
