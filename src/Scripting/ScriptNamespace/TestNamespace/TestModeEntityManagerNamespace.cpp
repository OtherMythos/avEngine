#ifdef TEST_MODE

#include "TestModeEntityManagerNamespace.h"
#include "System/BaseSingleton.h"

#include "Entity/EntityManager.h"

namespace AV{
    SQInteger TestModeEntityManagerNamespace::getEntityCount(HSQUIRRELVM vm){

        {
            EntityManager::EntityDebugInfo i;
            BaseSingleton::getEntityManager()->getDebugInfo(&i);

            sq_pushinteger(vm, i.totalEntities);
        }
        return 1;
    }

    SQInteger TestModeEntityManagerNamespace::getLoadedCallbackScriptCount(HSQUIRRELVM vm){

        {
            EntityManager::EntityDebugInfo i;
            BaseSingleton::getEntityManager()->getDebugInfo(&i);

            sq_pushinteger(vm, i.totalCallbackScripts);
        }
        return 1;
    }

    void TestModeEntityManagerNamespace::setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled){
        ScriptUtils::RedirectFunctionMap functionMap;
        functionMap["getEntityCount"] = {"", 0, getEntityCount};
        functionMap["getLoadedCallbackScriptCount"] = {"", 0, getLoadedCallbackScriptCount};

        ScriptUtils::redirectFunctionMap(vm, messageFunction, functionMap, testModeEnabled);
    }
}

#endif
