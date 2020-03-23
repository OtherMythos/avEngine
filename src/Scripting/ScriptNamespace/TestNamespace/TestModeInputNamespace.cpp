#ifdef TEST_MODE

#include "TestModeInputNamespace.h"

#include "System/BaseSingleton.h"
#include "Input/InputManager.h"

#include "Scripting/ScriptNamespace/InputNamespace.h"

namespace AV{

    SQInteger TestModeInputNamespace::sendButtonAction(HSQUIRRELVM vm){
        SQBool outBool;
        sq_getbool(vm, -1, &outBool);

        ActionHandle handle;
        SQInteger result = InputNamespace::_readActionHandle(vm, -2, &handle);
        if(result < 0) return result;

        BaseSingleton::getInputManager()->setButtonAction(0, handle, outBool);

        return 0;
    }

    SQInteger TestModeInputNamespace::sendTriggerAction(HSQUIRRELVM vm){
        SQFloat outFloat;
        sq_getfloat(vm, -1, &outFloat);

        ActionHandle handle;
        SQInteger result = InputNamespace::_readActionHandle(vm, -2, &handle);
        if(result < 0) return result;

        BaseSingleton::getInputManager()->setAnalogTriggerAction(0, handle, outFloat);

        return 0;
    }

    SQInteger TestModeInputNamespace::sendAxisAction(HSQUIRRELVM vm){
        SQBool outBool;
        sq_getbool(vm, -1, &outBool);
        SQFloat outFloat;
        sq_getfloat(vm, -2, &outFloat);

        ActionHandle handle;
        SQInteger result = InputNamespace::_readActionHandle(vm, -3, &handle);
        if(result < 0) return result;

        BaseSingleton::getInputManager()->setAxisAction(0, handle, outBool, outFloat);

        return 0;
    }

    void TestModeInputNamespace::setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled){
        ScriptUtils::RedirectFunctionMap functionMap;
        functionMap["sendButtonAction"] = {".ub", 3, sendButtonAction};
        functionMap["sendTriggerAction"] = {".uf", 3, sendTriggerAction};
        functionMap["sendAxisAction"] = {".ufb", 4, sendTriggerAction};

        ScriptUtils::redirectFunctionMap(vm, messageFunction, functionMap, testModeEnabled);
    }
}

#endif
