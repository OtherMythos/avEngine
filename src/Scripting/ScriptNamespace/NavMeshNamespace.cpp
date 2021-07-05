#include "NavMeshNamespace.h"

#include "World/WorldSingleton.h"
#include "World/Nav/NavMeshManager.h"
#include "Scripting/ScriptNamespace/Classes/Nav/NavMeshUserData.h"

namespace AV{

    SQInteger NavMeshNamespace::getNumMeshes(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        SQInteger num = world->getNavMeshManager()->getNumNavMeshes();
        sq_pushinteger(vm, num);

        return 1;
    }

    SQInteger NavMeshNamespace::getMeshByName(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        const SQChar *name;
        sq_getstring(vm, -1, &name);

        NavMeshId foundId = world->getNavMeshManager()->getMeshByName(name);

        NavMeshUserData::meshToUserData(vm, foundId);

        return 1;
    }

    /**SQNamespace
    @name _nav
    @desc Namespace to obtain information on the current nav meshes.
    */
    void NavMeshNamespace::setupNamespace(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, getNumMeshes, "getNumMeshes");
        ScriptUtils::addFunction(vm, getMeshByName, "getMeshByName");
    }
}
