#include "TestModeEntityManagerNamespace.h"

#include "World/WorldSingleton.h"
#include "World/Entity/EntityManager.h"

namespace AV{
    SQInteger TestModeEntityManagerNamespace::getEntityCount(HSQUIRRELVM vm){
        World* world = WorldSingleton::getWorld();
        if(world){
            EntityManager::EntityDebugInfo i;
            WorldSingleton::getWorld()->getEntityManager()->getDebugInfo(&i);

            sq_pushinteger(vm, i.totalEntities);
            return 1;
        }
        return 0;
    }

    SQInteger TestModeEntityManagerNamespace::getTrackedEntityCount(HSQUIRRELVM vm){
        World* world = WorldSingleton::getWorld();
        if(world){
            EntityManager::EntityDebugInfo i;
            WorldSingleton::getWorld()->getEntityManager()->getDebugInfo(&i);

            sq_pushinteger(vm, i.trackedEntities);
            return 1;
        }
        return 0;
    }

    void TestModeEntityManagerNamespace::setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled){
        RedirectFunctionMap functionMap;
        functionMap["getEntityCount"] = {"", 0, getEntityCount};
        functionMap["getTrackedEntityCount"] = {"", 0, getTrackedEntityCount};

        _redirectFunctionMap(vm, messageFunction, functionMap, testModeEnabled);
    }
}
