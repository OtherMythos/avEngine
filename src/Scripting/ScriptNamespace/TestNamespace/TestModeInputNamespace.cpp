#ifdef TEST_MODE

#include "TestModeInputNamespace.h"

#include "System/BaseSingleton.h"
#include "Input/InputManager.h"

#include "Scripting/ScriptNamespace/InputNamespace.h"
#include "Window/Window.h"
#include "Window/InputMapper.h"

namespace AV{

    SQInteger TestModeInputNamespace::sendButtonAction(HSQUIRRELVM vm){
        SQBool outBool;
        sq_getbool(vm, -1, &outBool);
        SQInteger deviceId;
        sq_getinteger(vm, -2, &deviceId);

        ActionHandle handle;
        SQInteger result = InputNamespace::readActionHandleUserData(vm, -3, &handle);
        if(result < 0) return result;

        BaseSingleton::getInputManager()->setButtonAction((InputDeviceId)deviceId, handle, outBool);

        return 0;
    }

    SQInteger TestModeInputNamespace::sendTriggerAction(HSQUIRRELVM vm){
        SQFloat outFloat;
        sq_getfloat(vm, -1, &outFloat);
        SQInteger deviceId;
        sq_getinteger(vm, -2, &deviceId);

        ActionHandle handle;
        SQInteger result = InputNamespace::readActionHandleUserData(vm, -3, &handle);
        if(result < 0) return result;

        BaseSingleton::getInputManager()->setAnalogTriggerAction((InputDeviceId)deviceId, handle, outFloat);

        return 0;
    }

    SQInteger TestModeInputNamespace::sendAxisAction(HSQUIRRELVM vm){
        SQBool outBool;
        sq_getbool(vm, -1, &outBool);
        SQInteger deviceId;
        sq_getinteger(vm, -2, &deviceId);
        SQFloat outFloat;
        sq_getfloat(vm, -3, &outFloat);

        ActionHandle handle;
        SQInteger result = InputNamespace::readActionHandleUserData(vm, -4, &handle);
        if(result < 0) return result;

        BaseSingleton::getInputManager()->setAxisAction((InputDeviceId)deviceId, handle, outBool, outFloat);

        return 0;
    }

    SQInteger TestModeInputNamespace::sendKeyboardKeyPress(HSQUIRRELVM vm){
        SQInteger idx;
        SQBool value;
        sq_getbool(vm, -1, &value);
        sq_getinteger(vm, -2, &idx);

        ActionHandle handle = BaseSingleton::getWindow()->getInputMapper()->getKeyboardMap(idx);
        BaseSingleton::getInputManager()->setKeyboardKeyAction(handle, value ? 1.0 : 0.0);

        return 0;
    }

    void TestModeInputNamespace::setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled){
        ScriptUtils::RedirectFunctionMap functionMap;
        functionMap["sendButtonAction"] = {".uib", 4, sendButtonAction};
        functionMap["sendTriggerAction"] = {".uif", 4, sendTriggerAction};
        functionMap["sendAxisAction"] = {".ufib", 5, sendAxisAction};
        functionMap["sendKeyboardKeyPress"] = {".ib", 3, sendKeyboardKeyPress};

        ScriptUtils::redirectFunctionMap(vm, messageFunction, functionMap, testModeEnabled);
    }
}

#endif
