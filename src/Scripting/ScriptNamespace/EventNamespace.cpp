#include "EventNamespace.h"

#include "ScriptGetterUtils.h"
#include "Scripting/ScriptNamespace/Classes/QuaternionUserData.h"

namespace AV{

    SQInteger EventNamespace::subscribe(HSQUIRRELVM vm){

        return 0;
    }

    /**SQNamespace
    @name _event
    @desc Namespace for registering functions to events
    */
    void EventNamespace::setupNamespace(HSQUIRRELVM vm){

        ScriptUtils::addFunction(vm, subscribe, "subscribe");
    }
}
