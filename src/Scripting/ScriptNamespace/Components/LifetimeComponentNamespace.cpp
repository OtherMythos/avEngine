#include "LifetimeComponentNamespace.h"

#include "World/Entity/Logic/LifetimeComponentLogic.h"
#include "Scripting/ScriptNamespace/Classes/EntityClass/EntityClass.h"

namespace AV{

    SQInteger LifetimeComponentNamespace::add(HSQUIRRELVM vm){
        SQInteger lifetime;
        sq_getinteger(vm, -1, &lifetime);

        eId id;
        SCRIPT_CHECK_RESULT(EntityClass::getEID(vm, -2, &id));

        LifetimeComponentLogic::add(id, lifetime);

        return 0;
    }

    SQInteger LifetimeComponentNamespace::remove(HSQUIRRELVM vm){
        eId id;
        SCRIPT_CHECK_RESULT(EntityClass::getEID(vm, -1, &id));

        LifetimeComponentLogic::remove(id);

        return 0;
    }

    void LifetimeComponentNamespace::setupNamespace(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("lifetime"), -1);
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, add, "add", 3, ".xi");
        ScriptUtils::addFunction(vm, remove, "remove", 2, ".x");

        sq_newslot(vm, -3, false);
    }
}
