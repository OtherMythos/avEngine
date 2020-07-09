#ifdef TEST_MODE

#include "TestModeEntityManagerNamespace.h"

#include "World/WorldSingleton.h"
#include "World/Entity/EntityManager.h"

namespace AV{
    SQInteger TestModeEntityManagerNamespace::getEntityCount(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            EntityManager::EntityDebugInfo i;
            WorldSingleton::getWorld()->getEntityManager()->getDebugInfo(&i);

            sq_pushinteger(vm, i.totalEntities);
        }
        return 1;
    }

    SQInteger TestModeEntityManagerNamespace::getTrackedEntityCount(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            EntityManager::EntityDebugInfo i;
            WorldSingleton::getWorld()->getEntityManager()->getDebugInfo(&i);

            sq_pushinteger(vm, i.trackedEntities);
        }
        return 1;
    }

    SQInteger TestModeEntityManagerNamespace::getLoadedCallbackScriptCount(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            EntityManager::EntityDebugInfo i;
            WorldSingleton::getWorld()->getEntityManager()->getDebugInfo(&i);

            sq_pushinteger(vm, i.totalCallbackScripts);
        }
        return 1;
    }

    void TestModeEntityManagerNamespace::setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled){
        ScriptUtils::RedirectFunctionMap functionMap;
        functionMap["getEntityCount"] = {"", 0, getEntityCount};
        functionMap["getTrackedEntityCount"] = {"", 0, getTrackedEntityCount};
        functionMap["getLoadedCallbackScriptCount"] = {"", 0, getLoadedCallbackScriptCount};

        ScriptUtils::redirectFunctionMap(vm, messageFunction, functionMap, testModeEnabled);
    }
}

#endif
