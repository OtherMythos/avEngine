#pragma once

#include "Event.h"

namespace AV{
    class WorldEvent : public Event{
    public:
        //TODO try and make this avoid the constructor setup.
        WorldEvent(){
            this->type = EventType::world;
        }

        int something = 10;
    };
}
