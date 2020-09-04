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

    void ScriptEventManager::unsubscribeEvent(EventId event){
        assert(event != EventId::Null);

        auto it = mSubscribeMap.find(event);
        if(it == mSubscribeMap.end()){
            //There was nothing subscribed.
            return;
        }

        ScriptVM::dereferenceObject(it->second.first);
        ScriptVM::dereferenceObject(it->second.second);

        mSubscribeMap.erase(it);
    }

    void ScriptEventManager::subscribeEvent(EventId event, SQObject closure, SQObject context){
        assert(event != EventId::Null);

        ScriptVM::referenceObject(closure);
        ScriptVM::referenceObject(context);

        auto it = mSubscribeMap.find(event);
        if(it == mSubscribeMap.end()){
            mSubscribeMap[event] = {closure, context};
            return;
        }

        //If it already contains an entry, the old one needs to be dereferenced.
        ScriptVM::dereferenceObject(it->second.first);
        ScriptVM::dereferenceObject(it->second.second);
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

    EventId calledId;
    SQInteger populateClosureCall(HSQUIRRELVM vm){
        sq_pushinteger(vm, static_cast<SQInteger>(calledId));

        //In future I will have a system to generate a table based on the provided data.
        sq_pushnull(vm);

        return 3;
    }

    void ScriptEventManager::_callQueuedEvent(const QueuedEventEntry& e){
        auto it = mSubscribeMap.find(e.id);
        //TODO I could consider reducing this so events aren't even queued unless they're subscribed.
        if(it == mSubscribeMap.end()) return;

        calledId = e.id;
        ScriptVM::callClosure(it->second.first, &(it->second.second), &populateClosureCall);
    }

    void ScriptEventManager::_queueEvent(const Event& e){

        mQueuedEvents.push({e.eventId(), 0});
    }

    inline bool _checkSQObject(const SQObject& a, const SQObject& b){
        return a._type == b._type && a._unVal.fFloat == b._unVal.fFloat;
    }
}