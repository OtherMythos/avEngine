#pragma once

#include <functional>
#include <vector>
#include <map>

#include "Events/Event.h"

namespace AV{

    #define AV_BIND(x) std::pair<std::function<bool(const Event&)>, void*>(std::bind(&x, this, std::placeholders::_1), this)

    class EventDispatcher{
    public:
        typedef std::function<bool(const Event&)> FunctionType;
        typedef std::pair<FunctionType, void*> EventFunction;

        static bool subscribe(EventType type, EventFunction function);
        static bool unsubscribe(EventType type, EventFunction function);

        static std::map< EventType, std::vector<EventFunction> > entryMap;

        static bool transmitEvent(EventType type, const Event &e);

    private:
        static bool _entryMapContains(EventType type, void* entryClass);
    };

}
