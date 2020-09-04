#include "EventNamespace.h"

#include "Scripting/Event/ScriptEventManager.h"

namespace AV{
    ScriptEventManager* EventNamespace::_scriptEventManager;

    SQInteger EventNamespace::subscribe(HSQUIRRELVM vm){
        SQInteger targetEvent;
        sq_getinteger(vm, 2, &targetEvent);

        SQObject targetClosure;
        sq_resetobject(&targetClosure);
        sq_getstackobj(vm, 3, &targetClosure);
        assert(targetClosure._type == OT_CLOSURE);
        {
            //Check the provided closure is of the correct format.
            SQInteger numParams, numFreeVariables;
            sq_getclosureinfo(vm, 3, &numParams, &numFreeVariables);
            if(numParams != 3) return sq_throwerror(vm, "Incorrect function format.");
        }

        SQObject targetContext;
        sq_resetobject(&targetContext);

        SQInteger size = sq_gettop(vm);
        if(size >= 4){
            sq_getstackobj(vm, 4, &targetContext);
        }

        EventId id = static_cast<EventId>(targetEvent);
        _scriptEventManager->subscribeEvent(id, targetClosure, targetContext);

        return 0;
    }

    /**SQNamespace
    @name _event
    @desc Namespace for registering functions to events
    */
    void EventNamespace::setupNamespace(HSQUIRRELVM vm){

        ScriptUtils::addFunction(vm, subscribe, "subscribe", -3, ".ict|x");
    }

    void EventNamespace::setupConstants(HSQUIRRELVM vm){
        for(size_t i = 0; i < static_cast<size_t>(EventId::EVENT_ID_END); i++){
            const char* c = EventIdStr[i];
            ScriptUtils::declareConstant(vm, c, i);
        }
    }
}
