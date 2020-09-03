#include "ScriptEventManager.h"

#include "Scripting/ScriptVM.h"
#include "Event/EventDispatcher.h"
#include <cassert>

namespace AV{
    ScriptEventManager::ScriptEventManager(){
        EventDispatcher::subscribe(EventType::World, AV_BIND(ScriptEventManager::eventReceiver));
        EventDispatcher::subscribe(EventType::Chunk, AV_BIND(ScriptEventManager::eventReceiver));
        EventDispatcher::subscribe(EventType::System, AV_BIND(ScriptEventManager::eventReceiver));
    }

    ScriptEventManager::~ScriptEventManager(){

    }

    void ScriptEventManager::subscribeEvent(EventId event, SQObject closure, SQObject context){
        assert(event != EventId::Null);

        //Check if the entry map already contains that function bound to that event.
        // if(_entryMapContains(event, function.second)) return false;

        // entryMap[type].push_back(function)
        // return true;

        auto it = mSubscribeMap.find(event);
        if(it == mSubscribeMap.end()){
            mSubscribeMap[event] = {closure, context};
            return;
        }

        //If it already contains an entry, the old one needs to be dereferenced.
        ScriptVM::dereferenceClosure(it->second.first);
        ScriptVM::dereferenceClosure(it->second.second);
        ScriptVM::referenceClosure(closure);
        ScriptVM::referenceClosure(context);
        mSubscribeMap[event] = {closure, context};

    }

    bool ScriptEventManager::eventReceiver(const Event &e){
        _queueEvent(e);

        return true;
    }

    void ScriptEventManager::processQueuedEvents(){
        while(!mQueuedEvents.empty()){
            QueuedEventEntry e = mQueuedEvents.front();
            mQueuedEvents.pop();

            _callQueuedEvent(e);
        }
    }

    void ScriptEventManager::_callQueuedEvent(const QueuedEventEntry& e){
        auto it = mSubscribeMap.find(e.id);
        //TODO I could consider reducing this so events aren't even queued unless they're subscribed.
        if(it == mSubscribeMap.end()) return;

        ScriptVM::callClosure(it->second.first, &(it->second.second));
    }

    void ScriptEventManager::_queueEvent(const Event& e){

        mQueuedEvents.push({e.eventId(), 0});
    }

    inline bool _checkSQObject(const SQObject& a, const SQObject& b){
        return a._type == b._type && a._unVal.fFloat == b._unVal.fFloat;
    }
}