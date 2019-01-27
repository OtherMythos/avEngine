#include "EventDispatcher.h"

namespace AV{
    std::map<EventType, std::vector<std::function<bool(const Event&)>>> EventDispatcher::entryMap;

    bool EventDispatcher::transmitEvent(EventType type, const Event &e){
        if(e.type != type) return false;

        for(auto a : entryMap[type]){
            a(e);
        }
        return true;
    }

    void EventDispatcher::subscribe(EventType type, std::function<bool(const Event&)> function){
        entryMap[type].push_back(function);
    }
}
