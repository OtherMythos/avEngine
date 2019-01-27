#pragma once

#include <functional>
#include <vector>
#include <map>

#include "Events/Event.h"

namespace AV{

    #define AV_BIND(x) std::bind(&x, this, std::placeholders::_1)

    class EventDispatcher{
    public:
        static void subscribe(EventType type, std::function<bool(const Event&)> function);

        static std::map< EventType, std::vector<std::function<bool(const Event&)>> > entryMap;

        static bool transmitEvent(EventType type, const Event &e);
    };

}
