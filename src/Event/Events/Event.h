#pragma once

namespace AV{
    #define AV_EVENT_TYPE(eType) virtual EventType type() const { return eType; }
    
    enum class EventType{
        Null = 0,
        World
    };

    class Event{
    public:
        AV_EVENT_TYPE(EventType::Null)
    };
};
