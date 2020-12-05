#include "EventNamespace.h"

#include "Scripting/Event/ScriptEventManager.h"

namespace AV{
    ScriptEventManager* EventNamespace::_scriptEventManager;

    SQInteger EventNamespace::subscribe(HSQUIRRELVM vm){
        SQInteger targetEvent;
        sq_getinteger(vm, 2, &targetEvent);
        EventId id = static_cast<EventId>(targetEvent);
        if(id == EventId::Null) return sq_throwerror(vm, "Null events cannot be subscribed to.");

        const bool userEvent = targetEvent > 1000;

        SQObject targetClosure;
        sq_resetobject(&targetClosure);
        sq_getstackobj(vm, 3, &targetClosure);
        //Assuming the param check worked.
        assert(targetClosure._type == OT_CLOSURE);

        //Check the provided closure is of the correct format.
        SQInteger numParams, numFreeVariables;
        sq_getclosureinfo(vm, 3, &numParams, &numFreeVariables);
        if(numParams != 3) return sq_throwerror(vm, "Subscribed function must take 2 parameters.");

        SQObject targetContext;
        sq_resetobject(&targetContext);

        SQInteger size = sq_gettop(vm);
        if(size >= 4){
            sq_getstackobj(vm, 4, &targetContext);
        }

        if(userEvent){
            _scriptEventManager->subscribeEvent(static_cast<int>(targetEvent), targetClosure, targetContext);
        }else{
            _scriptEventManager->subscribeEvent(id, targetClosure, targetContext);
        }

        return 0;
    }

    SQInteger EventNamespace::unsubscribe(HSQUIRRELVM vm){
        SQInteger targetEvent;
        sq_getinteger(vm, 2, &targetEvent);
        EventId id = static_cast<EventId>(targetEvent);
        if(id == EventId::Null) return sq_throwerror(vm, "Null events cannot be unsubscribed to.");

        if(targetEvent > 1000){
            SQObject targetClosure;
            sq_resetobject(&targetClosure);
            SQInteger size = sq_gettop(vm);
            if(size >= 3){
                sq_getstackobj(vm, 3, &targetClosure);
            }else return sq_throwerror(vm, "For user events a closure must be provided.");

            bool unsubscribed = _scriptEventManager->unsubscribeEvent(static_cast<int>(targetEvent), targetClosure);
            if(!unsubscribed) return sq_throwerror(vm, "No subscribed closure could be found.");
            return 0;
        }

        bool ubsubscribed = _scriptEventManager->unsubscribeEvent(id);
        if(!ubsubscribed){
            return sq_throwerror(vm, "No closure was subscribed to that event type.");
        }

        return 0;
    }

    SQInteger EventNamespace::transmit(HSQUIRRELVM vm){
        SQInteger targetEvent;
        sq_getinteger(vm, 2, &targetEvent);

        SQObject providedData;
        sq_resetobject(&providedData);
        sq_getstackobj(vm, 3, &providedData);

        _scriptEventManager->transmitEvent(static_cast<int>(targetEvent), providedData);

        return 0;
    }

    /**SQNamespace
    @name _event
    @desc Namespace for registering functions to events
    */
    void EventNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name subscribe
        @param1:EventId: The id of the target event. If the value is greater than 1000 it will be registered as a user event.
        @param2:Closure: The closure which should be called.
        @desc Subscribe a closure to an event. The provided closure will be called when the event occurs. As well as this, the user is able to provide a context in which the closure will be called.
        */
        ScriptUtils::addFunction(vm, subscribe, "subscribe", -3, ".ict|x");
        /**SQFunction
        @name unsubscribe
        @param1:EventId: The id of the target event. For this version, the id must be a system id, meaning it must be less than 1000.
        @desc Unsubscribe from an event.
        */
        /**SQFunction
        @name unsubscribe
        @param1:EventId: The id of the target event. If the value is greater than 1000 this is assumed to be a user event.
        @param2:Closure: If this is a user event, the target closure must be provided.
        @desc Unsubscribe a closure to an event. If no closure is provided an error will be thrown.
        */
        ScriptUtils::addFunction(vm, unsubscribe, "unsubscribe", -2, ".ic");
        /**SQFunction
        @name transmit
        @param1:EventId: A user event id. This value must be greater than 1000 or an error will be thrown.
        @param2:Data: A generic data object which is sent as part of the subscription.
        @desc Subscribe a closure to an event. The provided closure will be called when the event occurs. As well as this, the user is able to provide a context in which the closure will be called.
        */
        ScriptUtils::addFunction(vm, transmit, "transmit", 3, ".i.");
    }

    void EventNamespace::setupConstants(HSQUIRRELVM vm){
        for(size_t i = 0; i < static_cast<size_t>(EventId::EVENT_ID_END); i++){
            const char* c = EventIdStr[i];
            ScriptUtils::declareConstant(vm, c, i);
        }
    }
}
