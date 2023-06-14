#include "InputNamespace.h"

#include "Input/InputManager.h"
#include "Scripting/ScriptObjectTypeTags.h"
#include "System/BaseSingleton.h"
#include "Window/Window.h"
#include "Window/InputMapper.h"

#include "Scripting/ScriptNamespace/Classes/Vector2UserData.h"

#include "InputNamespaceConstants.h"

namespace AV {

    SQInteger InputNamespace::getMouseX(HSQUIRRELVM vm){
        sq_pushinteger(vm, BaseSingleton::getInputManager()->getMouseX());

        return 1;
    }

    SQInteger InputNamespace::getMouseY(HSQUIRRELVM vm){
        sq_pushinteger(vm, BaseSingleton::getInputManager()->getMouseY());

        return 1;
    }

    SQInteger InputNamespace::getActualMouseX(HSQUIRRELVM vm){
        sq_pushinteger(vm, BaseSingleton::getInputManager()->getActualMouseX());

        return 1;
    }

    SQInteger InputNamespace::getActualMouseY(HSQUIRRELVM vm){
        sq_pushinteger(vm, BaseSingleton::getInputManager()->getActualMouseY());

        return 1;
    }

    SQInteger InputNamespace::getMouseWheelValue(HSQUIRRELVM vm){
        sq_pushinteger(vm, BaseSingleton::getInputManager()->getMouseWheel());

        return 1;
    }

    SQInteger InputNamespace::getMouseButton(HSQUIRRELVM vm){
        SQInteger mouseButton = 0;
        sq_getinteger(vm, -1, &mouseButton);

        bool result = BaseSingleton::getInputManager()->getMouseButton(mouseButton);

        sq_pushbool(vm, result);
        return 1;
    }

    SQInteger InputNamespace::getTouchPosition(HSQUIRRELVM vm){
        SQInteger fingerId = 0;
        sq_getinteger(vm, -1, &fingerId);

        float x, y;
        bool result = BaseSingleton::getInputManager()->getTouchPosition(fingerId, &x, &y);
        if(!result){
            return sq_throwerror(vm, (std::string("Could not find a touch with fingerId ") + std::to_string(fingerId)).c_str());
        }

        Vector2UserData::vector2ToUserData(vm, Ogre::Vector2(x, y));
        return 1;
    }

    SQInteger InputNamespace::rumbleInputDevice(HSQUIRRELVM vm){
        SQInteger deviceId;
        sq_getinteger(vm, 2, &deviceId);

        if(deviceId < 0 || deviceId >= MAX_INPUT_DEVICES) return sq_throwerror(vm, "Invalid deviceID.");

        SQFloat lowFreqStrength;
        sq_getfloat(vm, 3, &lowFreqStrength);
        SQFloat highFreqStrength;
        sq_getfloat(vm, 4, &highFreqStrength);
        SQInteger rumbleTimeMs;
        sq_getinteger(vm, 5, &rumbleTimeMs);
        if(rumbleTimeMs < 0) return sq_throwerror(vm, "Rumble time must be greater than 0.");

        BaseSingleton::getWindow()->rumbleInputDevice(static_cast<InputDeviceId>(deviceId), lowFreqStrength, highFreqStrength, static_cast<uint32>(rumbleTimeMs));

        return 0;
    }

    SQInteger _getActionHandleImpl(HSQUIRRELVM vm, ActionHandle(InputManager::*funcPtr)(const std::string& s)){
        const SQChar *actionName;
        sq_getstring(vm, -1, &actionName);

        ActionHandle handle = ((*(BaseSingleton::getInputManager().get())).*funcPtr)(actionName);
        if(handle == INVALID_ACTION_HANDLE) return sq_throwerror(vm, "Error retreiving action handle.");

        InputNamespace::createActionHandleUserData(vm, handle);

        return 1;
    }

    SQInteger InputNamespace::getButtonActionHandle(HSQUIRRELVM vm){
        return _getActionHandleImpl(vm, &InputManager::getButtonActionHandle);
    }

    SQInteger InputNamespace::getAxisActionHandle(HSQUIRRELVM vm){
        return _getActionHandleImpl(vm, &InputManager::getAxisActionHandle);
    }

    SQInteger InputNamespace::getTriggerActionHandle(HSQUIRRELVM vm){
        return _getActionHandleImpl(vm, &InputManager::getAnalogTriggerActionHandle);
    }

    void InputNamespace::createActionHandleUserData(HSQUIRRELVM vm, ActionHandle handle){
        ActionHandle* pointer = (ActionHandle*)sq_newuserdata(vm, sizeof(ActionHandle));
        *pointer = handle;

        sq_settypetag(vm, -1, ButtonActionHandleTypeTag);
    }

    SQInteger InputNamespace::readActionHandleUserData(HSQUIRRELVM vm, SQInteger idx, ActionHandle* outHandle){
        SQUserPointer pointer = 0;
        SQUserPointer typeTag = 0;
        sq_getuserdata(vm, idx, &pointer, &typeTag);
        if(!pointer) return sq_throwerror(vm, "Unable to read from action handle.");
        if(typeTag != ButtonActionHandleTypeTag) return sq_throwerror(vm, "Incorrect object passed as action handle.");

        ActionHandle* actionHandle = static_cast<ActionHandle*>(pointer);
        *outHandle = *actionHandle;

        return 0;
    }

    //Repeated code, so it can go here.
    SQInteger getActionGetValues(HSQUIRRELVM vm, InputDeviceId* deviceId, ActionHandle* outHandle, SQInteger* outInputType){
        SQInteger topIdx = sq_gettop(vm);

        //The default is the any input device.
        *deviceId = ANY_INPUT_DEVICE;
        *outInputType = static_cast<SQInteger>(INPUT_TYPE_ANY);
        if(topIdx >= 3){
            //Target input type
            SQInteger iType;
            sq_getinteger(vm, 3, &iType);
            *outInputType = iType;
        }
        if(topIdx == 4){
            //A device id is being provided.
            SQInteger dId;
            sq_getinteger(vm, 4, &dId);
            //If any invalid numbers are given then default to the any input device.
            if( (dId >= 0 && dId < MAX_INPUT_DEVICES) || dId == KEYBOARD_INPUT_DEVICE) *deviceId = (InputDeviceId)dId;
        }

        *outHandle = INVALID_ACTION_HANDLE;
        SQInteger readResult = InputNamespace::readActionHandleUserData(vm, 2, outHandle);

        return readResult;
    }

    SQInteger InputNamespace::getButtonAction(HSQUIRRELVM vm){
        InputDeviceId deviceId = 0;
        ActionHandle handle = 0;
        SQInteger inputType;
        SQInteger outVal = getActionGetValues(vm, &deviceId, &handle, &inputType);
        if(outVal != 0) return outVal;

        bool result = BaseSingleton::getInputManager()->getButtonAction(deviceId, handle, static_cast<InputTypes>(inputType));

        sq_pushbool(vm, result);
        return 1;
    }

    SQInteger InputNamespace::getTriggerAction(HSQUIRRELVM vm){
        InputDeviceId deviceId = 0;
        ActionHandle handle = 0;
        SQInteger inputType;
        SQInteger outVal = getActionGetValues(vm, &deviceId, &handle, &inputType);
        if(outVal != 0) return outVal;

        float result = BaseSingleton::getInputManager()->getTriggerAction(deviceId, handle);

        sq_pushfloat(vm, result);
        return 1;
    }

    SQInteger InputNamespace::_getAxisAction(HSQUIRRELVM vm, bool x){
        InputDeviceId deviceId = 0;
        ActionHandle handle = 0;
        SQInteger inputType;
        SQInteger outVal = getActionGetValues(vm, &deviceId, &handle, &inputType);
        if(outVal != 0) return outVal;

        float result = BaseSingleton::getInputManager()->getAxisAction(deviceId, handle, x);

        sq_pushfloat(vm, result);
        return 1;
    }

    SQInteger InputNamespace::getAxisActionX(HSQUIRRELVM vm){
        return _getAxisAction(vm, true);
    }

    SQInteger InputNamespace::getAxisActionY(HSQUIRRELVM vm){
        return _getAxisAction(vm, false);
    }

    void _parseActionSetType(HSQUIRRELVM vm, ActionSetHandle handle, ActionType actionType){
        bool firstEntry = true;
        sq_pushnull(vm);
        while(SQ_SUCCEEDED(sq_next(vm, -2))){
            SQObjectType objectType = sq_gettype(vm, -1);
            if(objectType != OT_STRING){
                sq_pop(vm, 2);
                continue;
            }

            const SQChar *key;
            sq_getstring(vm, -2, &key);
            const SQChar *val;
            sq_getstring(vm, -1, &val);

            //Currently I'm not using the localised string.
            //Here we've assumed this entry is valid, so add it into the list.
            BaseSingleton::getInputManager()->createAction(key, handle, actionType, firstEntry);

            firstEntry = false;

            sq_pop(vm, 2);
        }
        sq_pop(vm, 1);
    }

    SQInteger _parseActionSet(HSQUIRRELVM vm, ActionSetHandle handle){
        sq_pushnull(vm);
        while(SQ_SUCCEEDED(sq_next(vm, -2))){
            SQObjectType objectType = sq_gettype(vm, -1);
            if(objectType != OT_TABLE){
                sq_pop(vm, 2);
                continue;
            }

            const SQChar *key;
            sq_getstring(vm, -2, &key);

            ActionType targetType = ActionType::Unknown;
            if(strcmp(key, "StickPadGyro") == 0) targetType = ActionType::StickPadGyro;
            else if(strcmp(key, "AnalogTrigger") == 0) targetType = ActionType::AnalogTrigger;
            else if(strcmp(key, "Buttons") == 0) targetType = ActionType::Button;
            if(targetType == ActionType::Unknown){
                //This table is a dud so continue the iteration.
                sq_pop(vm, 2);
                continue;
            }

            _parseActionSetType(vm, handle, targetType);

            sq_pop(vm, 2);
        }
        sq_pop(vm, 1);

        return 0;
    }

    SQInteger InputNamespace::setActionSets(HSQUIRRELVM vm){
        BaseSingleton::getInputManager()->clearAllActionSets();

        int addedCount = 0;
        sq_pushnull(vm);  //null iterator
        while(SQ_SUCCEEDED(sq_next(vm, -2))){
            SQObjectType objectType = sq_gettype(vm, -1);

            //Inside this table, I expect a list of other tables. Each one represents an action set.
            if(objectType != OT_TABLE){
                sq_pop(vm, 2);
                continue;
            }

            const SQChar *key;
            sq_getstring(vm, -2, &key);

            ActionSetHandle handle = BaseSingleton::getInputManager()->createActionSet(key);

            _parseActionSet(vm, handle);

            sq_pop(vm, 2);
            addedCount++;
        }
        BaseSingleton::getWindow()->getInputMapper()->setNumActionSets(addedCount);

        sq_pop(vm, 1); //pop the null iterator.

        return 0;
    }

    SQInteger InputNamespace::getActionSetNames(HSQUIRRELVM vm){
        auto actionSets = BaseSingleton::getInputManager()->getActionSets();
        sq_newarray(vm, 0);
        for(const auto& e : actionSets){
            sq_pushstring(vm, e.actionSetName.c_str(), -1);
            sq_arrayinsert(vm, -2, 0);
        }

        return 1;
    }

    void InputNamespace::createActionSetHandleUserData(HSQUIRRELVM vm, ActionSetHandle handle){
        ActionSetHandle* pointer = (ActionSetHandle*)sq_newuserdata(vm, sizeof(ActionSetHandle));
        *pointer = handle;

        sq_settypetag(vm, -1, ActionSetHandleTypeTag);
    }

    ActionSetHandle InputNamespace::readActionSetHandle(HSQUIRRELVM vm, SQInteger idx){
        SQUserPointer pointer, typetag;
        if(!SQ_SUCCEEDED(sq_getuserdata(vm, idx, &pointer, &typetag))) return INVALID_ACTION_SET_HANDLE;

        if(typetag != ActionSetHandleTypeTag) return INVALID_ACTION_SET_HANDLE;

        ActionSetHandle* handlePtr = (ActionSetHandle*)pointer;
        return *handlePtr;
    }

    SQInteger InputNamespace::getActionSetHandle(HSQUIRRELVM vm){
        auto actionSets = BaseSingleton::getInputManager()->getActionSets();

        const SQChar *key;
        sq_getstring(vm, -1, &key);

        for(ActionSetHandle i = 0; i < actionSets.size(); i++){
            if(strcmp(actionSets[i].actionSetName.c_str(), key) == 0){
                createActionSetHandleUserData(vm, i);
                return 1;
            }
        }

        return sq_throwerror(vm, "No action set found.");
    }

    SQInteger InputNamespace::getActionNamesForSet(HSQUIRRELVM vm){
        ActionSetHandle setHandle = readActionSetHandle(vm, -2);
        if(setHandle == INVALID_ACTION_SET_HANDLE) return sq_throwerror(vm, "Invalid action set handle.");

        SQInteger typeIdx = 0;
        sq_getinteger(vm, -1, &typeIdx);

        auto actionSets = BaseSingleton::getInputManager()->getActionSets();
        auto actionSetData = BaseSingleton::getInputManager()->getActionSetData();

        const InputManager::ActionSetEntry& e = actionSets[setHandle];
        size_t startIdx = 0;
        size_t endIdx = 0;
        switch(typeIdx){
            case 0:{ //StickPadGyro
                startIdx = e.stickStart;
                endIdx = e.stickEnd;
                break;
            }
            case 1:{ //AnalogTrigger
                startIdx = e.analogTriggerStart;
                endIdx = e.analogTriggerEnd;
                break;
            }
            case 2:{ //Button
                startIdx = e.buttonStart;
                endIdx = e.buttonEnd;
                break;
            }
        }

        sq_newarray(vm, 0);
        for(int i = startIdx; i < endIdx; i++){
            sq_pushstring(vm, actionSetData[i].first.c_str(), -1);
            sq_arrayinsert(vm, -2, 0);
        }

        return 1;
    }

    SQInteger InputNamespace::getNumControllers(HSQUIRRELVM vm){
        sq_pushinteger(vm, BaseSingleton::getInputManager()->getNumberOfActiveControllers());

        return 1;
    }

    SQInteger InputNamespace::getDeviceName(HSQUIRRELVM vm){
        SQInteger idx;
        sq_getinteger(vm, -1, &idx);

        const char* deviceName = BaseSingleton::getInputManager()->getDeviceName((InputDeviceId)idx);
        sq_pushstring(vm, deviceName, -1);

        return 1;
    }

    SQInteger InputNamespace::setActionSetForDevice(HSQUIRRELVM vm){
        SQInteger deviceId;
        sq_getinteger(vm, -2, &deviceId);
        InputDeviceId devId = INVALID_INPUT_DEVICE;
        if(deviceId == ANY_INPUT_DEVICE || (deviceId >= 0 && deviceId < MAX_INPUT_DEVICES) || deviceId == KEYBOARD_INPUT_DEVICE) devId = (InputDeviceId)deviceId;

        ActionSetHandle handle = readActionSetHandle(vm, -1);
        if(handle == INVALID_ACTION_SET_HANDLE) return sq_throwerror(vm, "Invalid action set handle.");

        BaseSingleton::getWindow()->getInputMapper()->setActionSetForDevice(devId, handle);

        return 0;
    }

    SQInteger InputNamespace::mapControllerInput(HSQUIRRELVM vm){
        SQInteger idx;
        sq_getinteger(vm, -2, &idx);

        ActionHandle handle = INVALID_ACTION_HANDLE;
        SQInteger readResult = readActionHandleUserData(vm, -1, &handle);
        if(readResult != 0) return readResult;

        bool result = BaseSingleton::getWindow()->getInputMapper()->mapControllerInput((int)idx, handle);
        if(!result) return sq_throwerror(vm, "Failed to map input to action handle.");

        return 0;
    }

    SQInteger InputNamespace::mapKeyboardInput(HSQUIRRELVM vm){
        SQInteger idx;
        sq_getinteger(vm, -2, &idx);

        ActionHandle handle = INVALID_ACTION_HANDLE;
        SQInteger readResult = readActionHandleUserData(vm, -1, &handle);
        if(readResult != 0) return readResult;

        bool result = BaseSingleton::getWindow()->getInputMapper()->mapKeyboardInput((int)idx, handle);
        if(!result) return sq_throwerror(vm, "Failed to map input to action handle.");

        return 0;
    }

    SQInteger InputNamespace::mapKeyboardInputAxis(HSQUIRRELVM vm){
        SQInteger posX, posY, negX, negY;
        sq_getinteger(vm, -2, &negY);
        sq_getinteger(vm, -3, &negX);
        sq_getinteger(vm, -4, &posY);
        sq_getinteger(vm, -5, &posX);

        ActionHandle handle = INVALID_ACTION_HANDLE;
        SQInteger readResult = readActionHandleUserData(vm, -1, &handle);
        if(readResult != 0) return readResult;

        bool result = BaseSingleton::getWindow()->getInputMapper()->mapKeyboardAxis((int)posX, (int)posY, (int)negX, (int)negY, handle);
        if(!result) return sq_throwerror(vm, "Failed to map input to action handle.");

        return 0;
    }

    SQInteger InputNamespace::getMostRecentDevice(HSQUIRRELVM vm){
        char value = BaseSingleton::getInputManager()->getMostRecentDevice();
        sq_pushinteger(vm, static_cast<SQInteger>(value));

        return 1;
    }

    SQInteger InputNamespace::clearAllMapping(HSQUIRRELVM vm){
        BaseSingleton::getWindow()->getInputMapper()->clearAllMapping();

        return 0;
    }


    SQInteger InputNamespace::sendButtonAction(HSQUIRRELVM vm){
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

    SQInteger InputNamespace::sendTriggerAction(HSQUIRRELVM vm){
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

    SQInteger InputNamespace::sendAxisAction(HSQUIRRELVM vm){
        SQBool outBool;
        sq_getbool(vm, -1, &outBool);
        SQFloat outFloat;
        sq_getfloat(vm, -2, &outFloat);
        SQInteger deviceId;
        sq_getinteger(vm, -3, &deviceId);

        ActionHandle handle;
        SQInteger result = InputNamespace::readActionHandleUserData(vm, -4, &handle);
        if(result < 0) return result;

        BaseSingleton::getInputManager()->setAxisAction((InputDeviceId)deviceId, handle, outBool, outFloat);

        return 0;
    }

    SQInteger InputNamespace::sendKeyboardKeyPress(HSQUIRRELVM vm){
        SQInteger idx;
        SQBool value;
        sq_getbool(vm, -1, &value);
        sq_getinteger(vm, -2, &idx);

        ActionHandle handle = BaseSingleton::getWindow()->getInputMapper()->getKeyboardMap(idx);
        BaseSingleton::getInputManager()->setKeyboardKeyAction(handle, value ? 1.0 : 0.0);

        return 0;
    }

    /**SQNamespace
    @name _input
    @desc This namespace provides functionality to retreive input.
    */
    void InputNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name getMouseX
        @returns The mouse x coordinate, relative to the top left of the window.
        */
        ScriptUtils::addFunction(vm, getMouseX, "getMouseX");
        /**SQFunction
        @name getMouseY
        @returns The mouse y coordinate, relative to the top left of the window.
        */
        ScriptUtils::addFunction(vm, getMouseY, "getMouseY");
        /**SQFunction
        @name getMouseWheelValue
        @returns Obtain the current delta values of the mouse wheel.
        */
        ScriptUtils::addFunction(vm, getMouseWheelValue, "getMouseWheelValue");
        /**SQFunction
        @name getMouseButton
        @param1:buttonId: The id of the mouse button to retreive.
        @returns True or false depending on whether the mouse button is down.
        */
        ScriptUtils::addFunction(vm, getMouseButton, "getMouseButton", 2, ".i");

        /**SQFunction
        @name getButtonActionHandle
        @param1:handleName: The name of the button action. Names must be unique across action sets.
        @returns A handle to a button action. This can later be used to retrieve input values.
        */
        ScriptUtils::addFunction(vm, getButtonActionHandle, "getButtonActionHandle", 2, ".s");
        /**SQFunction
        @name getAxisActionHandle
        @param1:handleName: The name of the axis action. Names must be unique across action sets.
        @returns A handle to an axis action.
        */
        ScriptUtils::addFunction(vm, getAxisActionHandle, "getAxisActionHandle", 2, ".s");
        /**SQFunction
        @name getTriggerActionHandle
        @param1:handleName: The name of the trigger action. Names must be unique across action sets.
        @returns A handle to an trigger action.
        */
        ScriptUtils::addFunction(vm, getTriggerActionHandle, "getTriggerActionHandle", 2, ".s");

        /**SQFunction
        @name setActionSetForDevice
        @param1:handleName: An integer id for the target device. Controllers this will be a value between 0 and _MAX_INPUT_DEVICES. _KEYBOARD_INPUT_DEVICE can also be passed.
        @param2:actionSetHandle: An action set handle.
        @desc Set the action set for a particular device.
        */
        ScriptUtils::addFunction(vm, setActionSetForDevice, "setActionSetForDevice", 3, ".iu");

        /**SQFunction
        @name getButtonAction
        @param1:buttonActionHandle: A button action handle. This has to be a button handle.
        @param2:inputType: Filter what sort of values are returned. Types are _INPUT_ANY, _INPUT_PRESSED, _INPUT_RELEASED.
        @param3:deviceId: The device id to query. If none is provided the default any device will be used.
        @return True or false depending on whether that button has been pressed.
        */
        ScriptUtils::addFunction(vm, getButtonAction, "getButtonAction", -2, ".uii");
        /**SQFunction
        @name getTriggerAction
        @param1:triggerActionHandle: A trigger action handle.
        @param2:inputType: Filter what sort of values are returned. Types are _INPUT_ANY, _INPUT_PRESSED, _INPUT_RELEASED.
        @param3:deviceId: The device id to query. If none is provided the default any device will be used.
        @return A float in the range 0 - 1, representing how pressed the trigger is.
        */
        ScriptUtils::addFunction(vm, getTriggerAction, "getTriggerAction", -2, ".uii");
        /**SQFunction
        @name getAxisActionX
        @param1:axisActionHandle: An axis action handle.
        @param2:inputType: Filter what sort of values are returned. Types are _INPUT_ANY, _INPUT_PRESSED, _INPUT_RELEASED.
        @param3:deviceId: The device id to query. If none is provided the default any device will be used.
        @return A float value in the range -1 - 1 with 0 representing the middle of the axis.
        */
        ScriptUtils::addFunction(vm, getAxisActionX, "getAxisActionX", -2, ".uii");
        /**SQFunction
        @name getAxisActionY
        @param1:axisActionHandle: An axis action handle.
        @param2:inputType: Filter what sort of values are returned. Types are _INPUT_ANY, _INPUT_PRESSED, _INPUT_RELEASED.
        @param3:deviceId: The device id to query. If none is provided the default any device will be used.
        @return A float value in the range -1 - 1 with 0 representing the middle of the axis.
        */
        ScriptUtils::addFunction(vm, getAxisActionY, "getAxisActionY", -2, ".uii");

        /**SQFunction
        @name setActionSets
        @param1:actionSets: A table containing the action sets.
        @desc Set the action sets which can be queried.
        */
        ScriptUtils::addFunction(vm, setActionSets, "setActionSets", 2, ".t");
        /**SQFunction
        @name getActionSetNames
        @returns A list containing all available action sets by name.
        */
        ScriptUtils::addFunction(vm, getActionSetNames, "getActionSetNames");
        /**SQFunction
        @name getActionSetHandle
        @desc Get a handle to an action set via name.
        @param1:actionSetName: A string representing an action set. The action set name must be unique.
        @returns A handle representing an action set.
        */
        ScriptUtils::addFunction(vm, getActionSetHandle, "getActionSetHandle", 2, ".s");
        /**SQFunction
        @name getActionNamesForSet
        @desc Get a list of action names for an action set.
        @param1:actionSetHandle: The target action set.
        @param2:type: An integer representing the action type to query. 0 is an axis, 1 is a trigger, 2 is a button.
        @returns A list of strings representing the actions.
        */
        ScriptUtils::addFunction(vm, getActionNamesForSet, "getActionNamesForSet", 3, ".ui");

        /**SQFunction
        @name getNumControllers
        @desc Get the number of controllers currently connected.
        @returns An integer representing the current number of controllers.
        */
        ScriptUtils::addFunction(vm, getNumControllers, "getNumControllers");
        /**SQFunction
        @name getDeviceName
        @desc Get the name of an attached input device.
        @param1:deviceId: The device to query.
        @returns A string representing the device name.
        */
        ScriptUtils::addFunction(vm, getDeviceName, "getDeviceName", 2, ".i");

        /**SQFunction
        @name mapControllerInput
        @desc Map an input id to an action.
        @param1:inputId: The target input to map. These values are different depending on the action handle.
        @param2:actionHandle: The target action handle to be bound.
        */
        ScriptUtils::addFunction(vm, mapControllerInput, "mapControllerInput", 3, ".iu");
        /**SQFunction
        @name mapKeyboardInput
        @desc Map a keyboard button to an action. This works for both button and trigger actions.
        */
        ScriptUtils::addFunction(vm, mapKeyboardInput, "mapKeyboardInput", 3, ".iu");
        /**SQFunction
        @name mapKeyboardInputAxis
        @desc Map multiple keyboard buttons to an axis. For instance you might want to map the WASD keys to an axis.
        @param1:posX: The key to assign to the positive x axis
        @param2:posY: The key to assign to the positive y axis.
        @param3:negX: The key to assign to the negative x axis.
        @param4:negY: The key to assign to the negative y axis.
        @param5:actionHandle: The handle to bind to the axis.
        */
        ScriptUtils::addFunction(vm, mapKeyboardInputAxis, "mapKeyboardInputAxis", 6, ".iiiiu");
        /**SQFunction
        @name clearAllMapping
        @desc Clear all the handles mapped to either keyboard or controller inputs.
        */
        ScriptUtils::addFunction(vm, clearAllMapping, "clearAllMapping");
        /**SQFunction
        @name getMostRecentDevice
        @desc Get the device which was used last frame.
        Can be useful to determine which devices are actually being used to interact with the project.
        @returns The most recent device which was used.
        If multiple last frame, this will be in the order keyboard, then controllers 0-MAX_INPUT_DEVICES.
        */
        ScriptUtils::addFunction(vm, getMostRecentDevice, "getMostRecentDevice");


        /**SQFunction
        @name sendButtonAction
        @desc Send a button action to the input system. Useful for spoofing inputs, i.e in onscreen controls.
        @param1:ActionHandle: The handle to send to.
        @param2:Integer: DeviceId.
        @param3:Boolean: Whether the button was pressed.
        */
        ScriptUtils::addFunction(vm, sendButtonAction, "sendButtonAction", 4, ".uib");
        /**SQFunction
        @name sendTriggerAction
        @desc Send a trigger action to the input system. Useful for spoofing inputs, i.e in onscreen controls.
        @param1:ActionHandle: The handle to send to.
        @param2:Integer: DeviceId.
        @param3:Number: The trigger value.
        */
        ScriptUtils::addFunction(vm, sendTriggerAction, "sendTriggerAction", 4, ".uin");
        /**SQFunction
        @name sendAxisAction
        @desc Send an axis action to the input system.  Useful for spoofing inputs, i.e in onscreen controls.
        @param1:ActionHandle: The handle to send to.
        @param2:Integer: DeviceId.
        @param3:Number: The axis value to set.
        @param4:Boolean: The axis value to set. True for x and false for y.
        */
        ScriptUtils::addFunction(vm, sendAxisAction, "sendAxisAction", 5, ".uinb");
        /**SQFunction
        @name sendKeyboardKeyPress
        @desc Send keyboard keypress to the input system.  Useful for spoofing inputs, i.e in onscreen controls.
        @param1:Integer: Key id to send the event to.
        @param2:Boolean: Whether or not the key is pressed.
        */
        ScriptUtils::addFunction(vm, sendKeyboardKeyPress, "sendKeyboardKeyPress", 3, ".ib");
        /**SQFunction
        @name getTouchPosition
        @desc Get the position of a touch with finger id. Details of touches occuring are delivered via system events.
        @param1:Integer: FingerId to respond to.
        */
        ScriptUtils::addFunction(vm, getTouchPosition, "getTouchPosition", 2, ".i");

        /**SQFunction
        @name rumbleInputDevice
        @desc Send a haptic rumble effect to the specified device. Only available for controllers.
        @param1:Integer: DeviceId. Must be between 0 and _MAX_INPUT_DEVICES
        @param1:Float: Decimal value representing Low frequency rumble strength.
        @param2:Float: Decimal value representing High frequency rumble strength.
        @param3:Integer: Rumble time in ms.
        */
        ScriptUtils::addFunction(vm, rumbleInputDevice, "rumbleInputDevice", 5, ".iffi");

    }

    void InputNamespace::setupConstants(HSQUIRRELVM vm){
        //So I ran into some problems with the const table.
        //For some reason I was unable to assign things in the const table and have that work.
        //It's more than likely something strange with my callback setup.
        //That should be investigated at some point in the future, but it's good enough for now.
        sq_pushroottable(vm);

        ScriptUtils::declareConstant(vm, "_MouseButtonLeft", 0);
        ScriptUtils::declareConstant(vm, "_MouseButtonRight", 1);

        ScriptUtils::declareConstant(vm, "_INPUT_ANY", INPUT_TYPE_ANY);
        ScriptUtils::declareConstant(vm, "_INPUT_PRESSED", INPUT_TYPE_PRESSED);
        ScriptUtils::declareConstant(vm, "_INPUT_RELEASED", INPUT_TYPE_RELEASED);

        ScriptUtils::declareConstant(vm, "_MAX_INPUT_DEVICES", MAX_INPUT_DEVICES);
        ScriptUtils::declareConstant(vm, "_KEYBOARD_INPUT_DEVICE", KEYBOARD_INPUT_DEVICE);
        ScriptUtils::declareConstant(vm, "_ANY_INPUT_DEVICE", ANY_INPUT_DEVICE);
        ScriptUtils::declareConstant(vm, "_INVALID_INPUT_DEVICE", INVALID_INPUT_DEVICE);


        for(size_t i = 0; i < sizeof(InputNamesKeys) / sizeof(const char*); i++){
            ScriptUtils::declareConstant(vm, InputNamesKeys[i], InputKeysValues[i]);
        }
        for(size_t i = 0; i < sizeof(InputButtonsNames) / sizeof(const char*); i++){
            ScriptUtils::declareConstant(vm, InputButtonsNames[i], i);
        }

        ScriptUtils::declareConstant(vm, "_BT_LEFT", 0);
        ScriptUtils::declareConstant(vm, "_BT_RIGHT", 1);
        ScriptUtils::declareConstant(vm, "_BA_LEFT", 0);
        ScriptUtils::declareConstant(vm, "_BA_RIGHT", 1);


        sq_pop(vm, 1); //pop the const table
    }
}
