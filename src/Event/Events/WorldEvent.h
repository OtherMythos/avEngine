#pragma once

#include "Event.h"

namespace AV{
    class WorldEvent : public Event{
    public:
        WorldEvent(){
            this->type = EventType::world;
        }

        int something = 10;
    };
}
