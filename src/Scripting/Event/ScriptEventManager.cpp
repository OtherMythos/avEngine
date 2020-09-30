#include "ScriptEventManager.h"

#include "Scripting/ScriptVM.h"
#include "Event/EventDispatcher.h"
#include "Event/Events/Script/ScriptEventDataFactory.h"
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

        if(!mSubscribedEventTypes[static_cast<size_t>(event)]){
            //There was nothing subscribed, so nothing to do.
            assert(mSubscribeMap.find(event) == mSubscribeMap.end());
            return;
        }

        auto it = mSubscribeMap.find(event);

        ScriptVM::dereferenceObject(it->second.first);
        ScriptVM::dereferenceObject(it->second.second);

        mSubscribeMap.erase(it);
        mSubscribedEventTypes[static_cast<size_t>(event)] = false;
    }

    void ScriptEventManager::subscribeEvent(EventId event, SQObject closure, SQObject context){
        assert(event != EventId::Null);

        ScriptVM::referenceObject(closure);
        ScriptVM::referenceObject(context);

        if(!mSubscribedEventTypes[static_cast<size_t>(event)]){
            //Nothing was subscribed previously.
            assert(mSubscribeMap.find(event) == mSubscribeMap.end());
            mSubscribedEventTypes[static_cast<size_t>(event)] = true;
            mSubscribeMap[event] = {closure, context};
            return;
        }

        auto it = mSubscribeMap.find(event);
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
    SQObject calledObject;
    SQInteger populateClosureCall(HSQUIRRELVM vm){
        sq_pushinteger(vm, static_cast<SQInteger>(calledId));

        sq_pushobject(vm, calledObject);

        return 3;
    }

    void ScriptEventManager::_callQueuedEvent(const QueuedEventEntry& e){
        auto it = mSubscribeMap.find(e.id);
        assert(mSubscribedEventTypes[static_cast<size_t>(e.id)]);

        calledId = e.id;
        calledObject = e.data;
        ScriptVM::callClosure(it->second.first, &(it->second.second), &populateClosureCall);
    }

    void ScriptEventManager::_queueEvent(const Event& e){
        //If no closure is registered for that event type there's nothing to do.
        if(!mSubscribedEventTypes[static_cast<size_t>(e.eventId())]) return;
        //TODO consider something other than friend direct access.
        SQObject obj = ScriptEventDataFactory::produceDataForEvent(ScriptVM::_sqvm, e);

        mQueuedEvents.push({e.eventId(), obj});
    }
}
