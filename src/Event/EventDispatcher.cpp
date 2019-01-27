#include "EventDispatcher.h"

namespace AV{
    std::map<EventType, std::vector<EventDispatcher::EventFunction>> EventDispatcher::entryMap;

    bool EventDispatcher::transmitEvent(EventType type, const Event &e){
        if(e.type != type) return false;

        for(auto a : entryMap[type]){
            a.first(e);
        }
        return true;
    }

    bool EventDispatcher::subscribe(EventType type, EventFunction function){
        if(type == EventType::Null) return false;
        if(function.first == nullptr) return false;

        //Check if the entry map already contains a binding for that class.
        if(_entryMapContains(type, function.second)) return false;

        entryMap[type].push_back(function);
        return true;
    }

    bool EventDispatcher::_entryMapContains(EventType type, void* entryClass){
        auto it = entryMap[type].begin();
        while(it != entryMap[type].end()){
            if((*it).second == entryClass) return true;
            it++;
        }
        return false;
    }
}
