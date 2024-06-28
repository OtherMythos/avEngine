#ifdef TEST_MODE

#include "TestModeInputNamespace.h"

#include "System/BaseSingleton.h"
#include "Input/InputManager.h"

#include "Scripting/ScriptNamespace/InputNamespace.h"
#include "Window/Window.h"
#include "Window/InputMapper.h"

namespace AV{

    SQInteger TestModeInputNamespace::sendControllerInput(HSQUIRRELVM vm){
        SQInteger devId, inputType, indexType;
        SQFloat value;
        sq_getfloat(vm, -1, &value);
        sq_getinteger(vm, -2, &indexType);
        sq_getinteger(vm, -3, &inputType);
        sq_getinteger(vm, -4, &devId);

        InputDeviceId deviceId = (InputDeviceId)devId;

        ActionHandle handle = 0;
        //0 axis, 1 trigger, anything else button.
        if(inputType == 0 || inputType == 1) {
            //I need to cast the values into something the system can understand.
            //This assumes you're using the SDL implementation.
            int targetAxis = 0;
            if(inputType == 1){
                if(indexType == 0) targetAxis = 4; //TriggerLeft
                else if(indexType == 1) targetAxis = 5; //TriggerRight
            }else{
                //I just take one of the axises. It's assumed they're both going to be mapped to the same handle anyway.
                if(indexType == 0) targetAxis = 0; //LeftStick
                else if(indexType == 1) targetAxis = 2; //RightStick
            }
            handle = BaseSingleton::getWindow()->getInputMapper()->getAxisMap(deviceId, targetAxis);
        }
        else handle = BaseSingleton::getWindow()->getInputMapper()->getButtonMap(deviceId, static_cast<int>(indexType));

        if(inputType == 0){
            BaseSingleton::getInputManager()->setAxisAction(deviceId, handle, true, value);
            BaseSingleton::getInputManager()->setAxisAction(deviceId, handle, false, value);
        }else if(inputType == 1){
            BaseSingleton::getInputManager()->setAnalogTriggerAction(deviceId, handle, value);
        }else{
            BaseSingleton::getInputManager()->setButtonAction(deviceId, handle, value > 0 ? 1.0 : 0.0);
        }

        return 0;
    }

    void TestModeInputNamespace::setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled){
        ScriptUtils::RedirectFunctionMap functionMap;
        functionMap["sendControllerInput"] = {".iiif", 5, sendControllerInput}; //Send a controller input. first i is the device id, second is the input type (axis, button, trigger), second is the index of the item, The float is the value to set. For axis this will be set for both directions.

        ScriptUtils::redirectFunctionMap(vm, messageFunction, functionMap, testModeEnabled);
    }
}

#endif
