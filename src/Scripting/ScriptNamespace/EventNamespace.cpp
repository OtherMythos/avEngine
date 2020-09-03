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
}
