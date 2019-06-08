#include "ScriptComponentNamespace.h"

#include "World/Entity/Logic/ScriptComponentLogic.h"
#include "Scripting/ScriptNamespace/Classes/EntityClass/EntityClass.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{

    SQInteger ScriptComponentNamespace::add(HSQUIRRELVM v){
        const SQChar *meshName;
        sq_getstring(v, -1, &meshName);

        ScriptComponentLogic::add(EntityClass::getEID(v, -2), Ogre::String(meshName));

        return 0;
    }

    SQInteger ScriptComponentNamespace::remove(HSQUIRRELVM v){
        eId id = EntityClass::getEID(v, -1);

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
