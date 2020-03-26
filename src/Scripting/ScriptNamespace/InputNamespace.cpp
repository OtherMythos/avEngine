#include "InputNamespace.h"

#include "Input/Input.h"
#include "Input/InputManager.h"
#include "Scripting/ScriptObjectTypeTags.h"
#include "System/BaseSingleton.h"
#include "Window/Window.h"
#include "Window/InputMapper.h"

namespace AV {
    SQInteger InputNamespace::getKey(HSQUIRRELVM vm){
        SQInteger key = 0;
        sq_getinteger(vm, -1, &key);

        SQBool result = Input::getKey((Input::Input_Key)key);

        sq_pushbool(vm, result);

        return 1;
    }

    SQInteger InputNamespace::getMouseX(HSQUIRRELVM vm){
        sq_pushinteger(vm, Input::getMouseX());

        return 1;
    }

    SQInteger InputNamespace::getMouseY(HSQUIRRELVM vm){
        sq_pushinteger(vm, Input::getMouseY());

        return 1;
    }

    SQInteger InputNamespace::getMouseButton(HSQUIRRELVM vm){
        SQInteger mouseButton = 0;
        sq_getinteger(vm, -1, &mouseButton);

        bool result = Input::getMouseButton(mouseButton);

        sq_pushbool(vm, result);
        return 1;
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

    SQInteger InputNamespace::getButtonAction(HSQUIRRELVM vm){
        SQInteger topIdx = sq_gettop(vm);

        //InputDeviceId deviceId = INVALID_INPUT_DEVICE;
        InputDeviceId deviceId = 0;
        if(topIdx == 3){
            //A device id is being provided.
            SQInteger dId;
            sq_getinteger(vm, 3, &dId);
            if(dId < 0 || dId >= MAX_INPUT_DEVICES) dId = 0;
            else deviceId = (InputDeviceId)dId;
        }

        ActionHandle handle = INVALID_ACTION_HANDLE;
        SQInteger readResult = readActionHandleUserData(vm, 2, &handle);
        if(readResult != 0) return readResult;

        bool result = BaseSingleton::getInputManager()->getButtonAction(deviceId, handle);

        sq_pushbool(vm, result);
        return 1;
    }

    SQInteger InputNamespace::getTriggerAction(HSQUIRRELVM vm){
        SQInteger topIdx = sq_gettop(vm);
        InputDeviceId deviceId = 0;
        if(topIdx == 3){
            SQInteger dId;
            sq_getinteger(vm, 3, &dId);
            if(dId < 0 || dId >= MAX_INPUT_DEVICES) dId = 0;
            else deviceId = (InputDeviceId)dId;
        }

        ActionHandle handle = INVALID_ACTION_HANDLE;
        SQInteger readResult = readActionHandleUserData(vm, 2, &handle);
        if(readResult != 0) return readResult;

        float result = BaseSingleton::getInputManager()->getTriggerAction(deviceId, handle);

        sq_pushfloat(vm, result);
        return 1;
    }

    SQInteger InputNamespace::_getAxisAction(HSQUIRRELVM vm, bool x){
        SQInteger topIdx = sq_gettop(vm);
        InputDeviceId deviceId = 0;
        if(topIdx == 3){
            //A device id is being provided.
            SQInteger dId;
            sq_getinteger(vm, 3, &dId);
            if(dId < 0 || dId >= MAX_INPUT_DEVICES) dId = 0;
            else deviceId = (InputDeviceId)dId;
        }

        ActionHandle handle = INVALID_ACTION_HANDLE;
        SQInteger readResult = readActionHandleUserData(vm, 2, &handle);
        if(readResult != 0) return readResult;

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
        }

        sq_pop(vm, 1); //pop the null iterator.

        return 0;
    }

    SQInteger InputNamespace::getActionSetNames(HSQUIRRELVM vm){
        auto outMap = BaseSingleton::getInputManager()->getActionSetMeta();

        sq_newarray(vm, 0);
        for(const auto& e : outMap){
            sq_pushstring(vm, e.first.c_str(), -1);
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
        auto outMap = BaseSingleton::getInputManager()->getActionSetMeta();

        const SQChar *key;
        sq_getstring(vm, -1, &key);

        for(const auto& e : outMap){
            if(strcmp(e.first.c_str(), key) == 0){
                createActionSetHandleUserData(vm, e.second);
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
        InputDeviceId devId = 0;
        if(deviceId >= 0 && deviceId < MAX_INPUT_DEVICES) devId = (InputDeviceId)deviceId;

        ActionSetHandle handle = readActionSetHandle(vm, -1);

        BaseSingleton::getWindow()->getInputMapper()->setActionSetForDevice(devId, handle);

        return 0;
    }

    void InputNamespace::setupNamespace(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, getKey, "getKey", 2, ".i");
        ScriptUtils::addFunction(vm, getMouseX, "getMouseX");
        ScriptUtils::addFunction(vm, getMouseY, "getMouseY");
        ScriptUtils::addFunction(vm, getMouseButton, "getMouseButton", 2, ".i");

        ScriptUtils::addFunction(vm, getButtonActionHandle, "getButtonActionHandle", 2, ".s");
        ScriptUtils::addFunction(vm, getAxisActionHandle, "getAxisActionHandle", 2, ".s");
        ScriptUtils::addFunction(vm, getTriggerActionHandle, "getTriggerActionHandle", 2, ".s");

        ScriptUtils::addFunction(vm, setActionSetForDevice, "setActionSetForDevice", 3, ".iu");

        ScriptUtils::addFunction(vm, getButtonAction, "getButtonAction", -2, ".ui");
        ScriptUtils::addFunction(vm, getTriggerAction, "getTriggerAction", -2, ".ui");
        ScriptUtils::addFunction(vm, getAxisActionX, "getAxisActionX", -2, ".ui");
        ScriptUtils::addFunction(vm, getAxisActionY, "getAxisActionY", -2, ".ui");

        ScriptUtils::addFunction(vm, setActionSets, "setActionSets", 2, ".t");
        ScriptUtils::addFunction(vm, getActionSetNames, "getActionSetNames");
        ScriptUtils::addFunction(vm, getActionSetHandle, "getActionSetHandle", 2, ".s");
        ScriptUtils::addFunction(vm, getActionNamesForSet, "getActionNamesForSet", 3, ".ui");

        ScriptUtils::addFunction(vm, getNumControllers, "getNumControllers");
        ScriptUtils::addFunction(vm, getDeviceName, "getDeviceName", 2, ".i");
    }

    void InputNamespace::setupConstants(HSQUIRRELVM vm){
        //So I ran into some problems with the const table.
        //For some reason I was unable to assign things in the const table and have that work.
        //It's more than likely something strange with my callback setup.
        //That should be investigated at some point in the future, but it's good enough for now.
        sq_pushroottable(vm);

        ScriptUtils::declareConstant(vm, "_KeyNull", Input::Key_Null);
        ScriptUtils::declareConstant(vm, "_KeyAccept", Input::Key_Accept);
        ScriptUtils::declareConstant(vm, "_KeyDecline", Input::Key_Decline);
        ScriptUtils::declareConstant(vm, "_KeyUp", Input::Key_Up);
        ScriptUtils::declareConstant(vm, "_KeyDown", Input::Key_Down);
        ScriptUtils::declareConstant(vm, "_KeyLeft", Input::Key_Left);
        ScriptUtils::declareConstant(vm, "_KeyRight", Input::Key_Right);
        ScriptUtils::declareConstant(vm, "_KeyDeveloperGuiToggle", Input::Key_DeveloperGuiToggle);

        ScriptUtils::declareConstant(vm, "_MouseButtonLeft", 0);
        ScriptUtils::declareConstant(vm, "_MouseButtonRight", 1);

        ScriptUtils::declareConstant(vm, "_MAX_INPUT_DEVICES", MAX_INPUT_DEVICES);

        sq_pop(vm, 1); //pop the const table
    }
}
