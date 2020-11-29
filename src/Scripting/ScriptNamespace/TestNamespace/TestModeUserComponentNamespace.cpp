#ifdef TEST_MODE

#include "TestModeUserComponentNamespace.h"
#include "System/SystemSetup/SystemSettings.h"

namespace AV{

    SQInteger TestModeUserComponentNamespace::getNumUserComponents(HSQUIRRELVM vm){
        const UserComponentSettings& s = SystemSettings::getUserComponentSettings();

        sq_pushinteger(vm, s.numRegisteredComponents);
        return 1;
    }

    SQInteger TestModeUserComponentNamespace::getUserComponentNames(HSQUIRRELVM vm){
        const UserComponentSettings& s = SystemSettings::getUserComponentSettings();

        sq_newarray(vm, 0);
        for(int i = 0; i < NUM_USER_COMPONENTS; i++){
            if(i >= s.numRegisteredComponents) break;
            sq_pushstring(vm, s.vars[i].componentName.c_str(), -1);
            sq_arrayinsert(vm, -2, 0);
        }

        return 1;
    }

    void TestModeUserComponentNamespace::setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled){
        ScriptUtils::RedirectFunctionMap functionMap;
        functionMap["getNumUserComponents"] = {"", 0, getNumUserComponents};
        functionMap["getUserComponentNames"] = {"", 0, getUserComponentNames};

        ScriptUtils::redirectFunctionMap(vm, messageFunction, functionMap, testModeEnabled);
    }
}

#endif
