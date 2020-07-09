#include "ScriptComponentNamespace.h"

#include "World/Entity/Logic/ScriptComponentLogic.h"
#include "Scripting/ScriptNamespace/Classes/EntityClass/EntityClass.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{

    SQInteger ScriptComponentNamespace::add(HSQUIRRELVM vm){
        const SQChar *meshName;
        sq_getstring(vm, -1, &meshName);

        eId id;
        SCRIPT_CHECK_RESULT(EntityClass::getEID(vm, -2, &id));

        ScriptComponentLogic::add(id, Ogre::String(meshName));

        return 0;
    }

    SQInteger ScriptComponentNamespace::remove(HSQUIRRELVM vm){
        eId id;
        SCRIPT_CHECK_RESULT(EntityClass::getEID(vm, -1, &id));

        ScriptComponentLogic::remove(id);

        return 0;
    }

    void ScriptComponentNamespace::setupNamespace(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("script"), -1);
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, add, "add", 3, ".xs");
        ScriptUtils::addFunction(vm, remove, "remove", 2, ".x");

        sq_newslot(vm, -3, false);
    }
}
