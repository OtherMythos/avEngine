#include "NavMeshNamespace.h"
#include "System/BaseSingleton.h"

#include "Nav/NavMeshManager.h"
#include "Scripting/ScriptNamespace/Classes/Nav/NavMeshUserData.h"
#include "Scripting/ScriptNamespace/Classes/Nav/NavMeshQueryUserData.h"

namespace AV{

    SQInteger NavMeshNamespace::getNumMeshes(HSQUIRRELVM vm){

        SQInteger num = BaseSingleton::getNavMeshManager()->getNumNavMeshes();
        sq_pushinteger(vm, num);

        return 1;
    }

    SQInteger NavMeshNamespace::getNumQueries(HSQUIRRELVM vm){

        SQInteger num = BaseSingleton::getNavMeshManager()->getNumQueries();
        sq_pushinteger(vm, num);

        return 1;
    }

    SQInteger NavMeshNamespace::getMeshByName(HSQUIRRELVM vm){

        const SQChar *name;
        sq_getstring(vm, -1, &name);

        NavMeshId foundId = BaseSingleton::getNavMeshManager()->getMeshByName(name);

        NavMeshUserData::meshToUserData(vm, foundId);

        return 1;
    }

    SQInteger NavMeshNamespace::createQuery(HSQUIRRELVM vm){

        NavMeshId outMesh;
        SCRIPT_CHECK_RESULT(NavMeshUserData::readMeshFromUserData(vm, 2, &outMesh));

        NavQueryId foundId = BaseSingleton::getNavMeshManager()->generateNavQuery(outMesh);

        NavMeshQueryUserData::queryToUserData(vm, foundId);

        return 1;
    }

    /**SQNamespace
    @name _nav
    @desc Namespace to obtain information on the current nav meshes.
    */
    void NavMeshNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name getNumMeshes
        @returns The number of meshes currently loaded.
        */
        ScriptUtils::addFunction(vm, getNumMeshes, "getNumMeshes");
        /**SQFunction
        @name getNumQueries
        @returns The number of active queries.
        */
        ScriptUtils::addFunction(vm, getNumQueries, "getNumQueries");
        /**SQFunction
        @name getMeshByName
        @desc Retreive a mesh object by name.
        @returns A handle to the requested mesh. If the mesh does not exist an invalid mesh handle is returned.
        @param1:String: The name of the mesh.
        */
        ScriptUtils::addFunction(vm, getMeshByName, "getMeshByName");
        /**SQFunction
        @name createQuery
        @desc Create a nav mesh query for a certain mesh. As an object this query can be re-used as many times as needed.
        @returns A nav mesh query instance.
        @param1:NavMesh: The nav mesh to base the query on.
        */
        ScriptUtils::addFunction(vm, createQuery, "createQuery", 2, ".u");
    }
}
