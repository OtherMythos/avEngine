#include "EventNamespace.h"

#include "Scripting/Event/ScriptEventManager.h"

namespace AV{
    ScriptEventManager* EventNamespace::_scriptEventManager;

    SQInteger EventNamespace::subscribe(HSQUIRRELVM vm){
        SQInteger targetEvent;
        sq_getinteger(vm, 2, &targetEvent);
        EventId id = static_cast<EventId>(targetEvent);
        if(id == EventId::Null) return sq_throwerror(vm, "Null events cannot be subscribed to.");

        SQObject targetClosure;
        sq_resetobject(&targetClosure);
        sq_getstackobj(vm, 3, &targetClosure);
        if(targetClosure._type == OT_CLOSURE){
            //Check the provided closure is of the correct format.
            SQInteger numParams, numFreeVariables;
            sq_getclosureinfo(vm, 3, &numParams, &numFreeVariables);
            if(numParams != 3) return sq_throwerror(vm, "Incorrect function format.");
        }
        else if(targetClosure._type == OT_NULL){
            //If null is passed this means unsubscribe.
            _scriptEventManager->unsubscribeEvent(id);
            return 0;
        }else{
            //Assuming the param check worked, it should never be anything else.
            assert(false);
        }

        SQObject targetContext;
        sq_resetobject(&targetContext);

        SQInteger size = sq_gettop(vm);
        if(size >= 4){
            sq_getstackobj(vm, 4, &targetContext);
        }

        _scriptEventManager->subscribeEvent(id, targetClosure, targetContext);

        return 0;
    }

    /**SQNamespace
    @name _event
    @desc Namespace for registering functions to events
    */
    void EventNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name subscribe
        @param1:EventId: The id of the target event.
        @param2:Closure: The closure which should be called. If null is provided this operates as an unsubscribe operation.
        @desc Subscribe a closure to an event. The provided closure will be called when the event occurs. As well as this, the user is able to provide a context in which the closure will be called.
        */
        ScriptUtils::addFunction(vm, subscribe, "subscribe", -3, ".ic|ot|x");
    }

    void EventNamespace::setupConstants(HSQUIRRELVM vm){
        for(size_t i = 0; i < static_cast<size_t>(EventId::EVENT_ID_END); i++){
            const char* c = EventIdStr[i];
            ScriptUtils::declareConstant(vm, c, i);
        }
    }
}
