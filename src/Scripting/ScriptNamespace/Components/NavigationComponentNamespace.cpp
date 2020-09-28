#include "NavigationComponentNamespace.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "World/Entity/Logic/NavigationComponentLogic.h"
#include "Scripting/ScriptNamespace/Classes/EntityClass/EntityClass.h"

namespace AV{

    SQInteger NavigationComponentNamespace::add(HSQUIRRELVM vm){
        eId id;
        SCRIPT_CHECK_RESULT(EntityClass::getEID(vm, -1, &id));

        NavigationComponentLogic::add(id);

        return 0;
    }

    SQInteger NavigationComponentNamespace::remove(HSQUIRRELVM vm){
        eId id;
        SCRIPT_CHECK_RESULT(EntityClass::getEID(vm, -1, &id));

        NavigationComponentLogic::remove(id);

        return 0;
    }

    void NavigationComponentNamespace::setupNamespace(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("navigation"), -1);
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, add, "add", 2, ".x");
        ScriptUtils::addFunction(vm, remove, "remove", 2, ".x");

        sq_newslot(vm, -3, false);
    }
}
