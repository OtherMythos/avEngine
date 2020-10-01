#include "NavMeshNamespace.h"

#include "World/WorldSingleton.h"
#include "World/Nav/NavMeshManager.h"

namespace AV{

    SQInteger NavMeshNamespace::getNumMeshes(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        SQInteger num = world->getNavMeshManager()->getNumNavMeshes();
        sq_pushinteger(vm, num);

        return 0;
    }

    /**SQNamespace
    @name _nav
    @desc Namespace to obtain information on the current nav meshes.
    */
    void NavMeshNamespace::setupNamespace(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, getNumMeshes, "getNumMeshes");
    }
}
