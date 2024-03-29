#include "EventDispatcher.h"

#include <cassert>

namespace AV{
    std::map<EventType, std::vector<EventDispatcher::EventFunction>> EventDispatcher::entryMap;
    std::map< EventType, std::vector<EventDispatcher::FunctionType> > EventDispatcher::mStaticEntryMap;

    bool EventDispatcher::transmitEvent(EventType type, const Event &e){
        assert(e.type() == type);

        //Regular listeners
        for(const auto& a : entryMap[type]){
            a.first(e);
        }

        //Static listeners
        for(const auto& a : mStaticEntryMap[type]){
            a(e);
        }

        return true;
    }

    bool EventDispatcher::subscribe(EventType type, EventFunction function){
        assert(type != EventType::Null);
        assert(function.first != nullptr);

        //Check if the entry map already contains a binding for that class.
        if(_entryMapContains(type, function.second)) return false;

        entryMap[type].push_back(function);
        return true;
    }

    bool EventDispatcher::subscribeStatic(EventType type, FunctionType function){
        assert(type != EventType::Null);

        if(_entryMapContains(type, function.target_type())) return false;

        mStaticEntryMap[type].push_back(function);

        return true;
    }

    bool EventDispatcher::unsubscribe(EventType type, void* ptr){
        assert(type != EventType::Null);
        assert(ptr != nullptr);

        auto it = entryMap[type].begin();
        while(it != entryMap[type].end()){
            if((*it).second == ptr){
                entryMap[type].erase(it);
                return true;
            }
            it++;
        }
        return false;
    }

    bool EventDispatcher::unsubscribeStatic(EventType type, FunctionType function){
        assert(type != EventType::Null);

        auto it = mStaticEntryMap[type].begin();
        const std::type_info& functionType = function.target_type();
        while(it != mStaticEntryMap[type].end()){
            if((*it).target_type() == functionType){
                mStaticEntryMap[type].erase(it);
                return true;
            }
            it++;
        }
        return false;
    }

    bool EventDispatcher::_entryMapContains(EventType type, const std::type_info& functionId){
        auto it = mStaticEntryMap[type].begin();
        while(it != mStaticEntryMap[type].end()){
            if((*it).target_type() == functionId) return true;
            it++;
        }
        return false;
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
