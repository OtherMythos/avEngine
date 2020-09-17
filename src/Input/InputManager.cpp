#include "InputManager.h"

#include <string.h>
#include "Logger/Log.h"
#include "Event/Events/SystemEvent.h"
#include "Event/EventDispatcher.h"

namespace AV{
    InputManager::InputManager(){
        for(int i = 0; i < MAX_INPUT_DEVICES; i++){
            _resetDeviceData(mDevices[i]);
        }

        mMouseX = 0;
        mMouseY = 0;
    }

    InputManager::~InputManager(){

    }

    void InputManager::setupDefaultActionSet(){
        clearAllActionSets();
        AV::ActionSetHandle handle = createActionSet("Default");

        createAction("LeftMove", handle, AV::ActionType::StickPadGyro, true);
        createAction("RightMove", handle, AV::ActionType::StickPadGyro, false);
        createAction("LeftTrigger", handle, AV::ActionType::AnalogTrigger, true);
        createAction("RightTrigger", handle, AV::ActionType::AnalogTrigger, false);
        createAction("Accept", handle, AV::ActionType::Button, true);
        createAction("Decline", handle, AV::ActionType::Button, false);
        createAction("Menu", handle, AV::ActionType::Button, false);
        createAction("Options", handle, AV::ActionType::Button, false);
        createAction("Start", handle, AV::ActionType::Button, false);
        createAction("Select", handle, AV::ActionType::Button, false);

        createAction("DirectionUp", handle, AV::ActionType::Button, false);
        createAction("DirectionDown", handle, AV::ActionType::Button, false);
        createAction("DirectionLeft", handle, AV::ActionType::Button, false);
        createAction("DirectionRight", handle, AV::ActionType::Button, false);
    }

    ActionSetHandle InputManager::createActionSet(const char* actionSetName){
        size_t size = mActionSets.size();
        assert(mActionSets.size() <= 255 && "No more than 255 action sets can be created.");
        ActionSetHandle handle = (ActionSetHandle)size;

        mActionSets.push_back({actionSetName, 0, 0, 0, 0, 0, 0}); //The 0 values will be populated later.

        return handle;
    }

    void InputManager::clearAllActionSets(){
        mActionSets.clear();
        mActionSetData.clear();

        for(int i = 0; i < MAX_INPUT_DEVICES; i++){
            mActionData[i].actionButtonData.clear();
            mActionData[i].actionAnalogTriggerData.clear();
            mActionData[i].actionStickPadGyroData.clear();
        }
        mKeyboardData.actionButtonData.clear();
        mKeyboardData.actionAnalogTriggerData.clear();
        mKeyboardData.actionStickPadGyroData.clear();
    }

    void InputManager::createAction(const char* actionName, ActionSetHandle actionSet, ActionType type, bool firstValue){
        assert(type != ActionType::Unknown);

        size_t* infoStart = 0;
        size_t* infoEnd = 0;
        size_t targetListSize = 0;
        ActionSetEntry& e = mActionSets[actionSet];
        switch(type){
            case ActionType::StickPadGyro:{
                //Taking the first index is fine as they should all have the same index.
                targetListSize = mActionData[0].actionStickPadGyroData.size();
                for(int i = 0; i < MAX_INPUT_DEVICES; i++)
                    mActionData[i].actionStickPadGyroData.push_back({0.0f, 0.0f});
                mKeyboardData.actionStickPadGyroData.push_back({0.0f, 0.0f});
                mAnyDeviceData.actionStickPadGyroData.push_back({0.0f, 0.0f});
                infoStart = &e.stickStart;
                infoEnd = &e.stickEnd;
                break;
            }
            case ActionType::AnalogTrigger:{
                targetListSize = mActionData[0].actionAnalogTriggerData.size();
                for(int i = 0; i < MAX_INPUT_DEVICES; i++)
                    mActionData[i].actionAnalogTriggerData.push_back(0.0f);
                mKeyboardData.actionAnalogTriggerData.push_back(0.0f);
                mAnyDeviceData.actionAnalogTriggerData.push_back(0.0f);
                infoStart = &e.analogTriggerStart;
                infoEnd = &e.analogTriggerEnd;
                break;
            }
            case ActionType::Button:{
                targetListSize = mActionData[0].actionButtonData.size();
                for(int i = 0; i < MAX_INPUT_DEVICES; i++)
                    mActionData[i].actionButtonData.push_back(false);
                mKeyboardData.actionButtonData.push_back(false);
                mAnyDeviceData.actionButtonData.push_back(false);
                infoStart = &e.buttonStart;
                infoEnd = &e.buttonEnd;
                break;
            }
            default: assert(false);
        }

        if(firstValue){
            *infoStart = mActionSetData.size();
            *infoEnd = mActionSetData.size();
        }
        mActionSetData.push_back({actionName, targetListSize});
        (*infoEnd)++;
    }

    InputDeviceId InputManager::addInputDevice(const char* deviceName){
        InputDeviceId targetId = 0;
        bool found = false;
        for(; targetId < MAX_INPUT_DEVICES; targetId++){
            if(!mDevices[targetId].populated){
                found = true;
                break;
            }
        }
        if(!found) return INVALID_INPUT_DEVICE; //There are no more available controllers.

        InputDeviceData& d = mDevices[targetId];

        strncpy(d.deviceName, deviceName, 20);
        d.populated = true;

        AV_INFO("Added a controller with id {}, as name '{}'", targetId, deviceName);
        mNumActiveControllers++;

        SystemEventInputDeviceAdded event;
        EventDispatcher::transmitEvent(EventType::System, event);

        return targetId;
    }

    bool InputManager::removeInputDevice(InputDeviceId dev){
        if(dev >= MAX_INPUT_DEVICES) return false;
        InputDeviceData& d = mDevices[dev];

        if(!d.populated) return false;
        AV_INFO("Removed controller with id {}, named '{}'", dev, d.deviceName);
        mNumActiveControllers--;

        _resetDeviceData(d);

        SystemEventInputDeviceRemoved event;
        EventDispatcher::transmitEvent(EventType::System, event);

        return true;
    }

    void InputManager::setCurrentActionSet(ActionSetHandle actionSet){
        mCurrentActionSet = actionSet;
    }

    ActionSetHandle InputManager::getActionSetHandle(const std::string& setName) const{
        for(ActionSetHandle i = 0; i < mActionSets.size(); i++){
            if(mActionSets[i].actionSetName == setName){
                return i;
            }
        }

        return INVALID_ACTION_SET_HANDLE;
    }

    ActionHandle InputManager::_getActionHandle(ActionType type, const std::string& actionName){
        assert(type != ActionType::Unknown);

        ActionSetHandle targetActionSet = 0;
        int targetIdx = -1;
        ActionType currentType = ActionType::Unknown;
        for(int y = 0; y < mActionSets.size(); y++){
            const ActionSetEntry& setEntry = mActionSets[y];
            for(int i = setEntry.buttonStart; i < setEntry.buttonEnd; i++){
                if(mActionSetData[i].first == actionName) { targetIdx = mActionSetData[i].second; currentType = ActionType::Button; break; }
            }
            if(targetIdx >= 0) break;
            for(int i = setEntry.analogTriggerStart; i < setEntry.analogTriggerEnd; i++){
                if(mActionSetData[i].first == actionName) { targetIdx = mActionSetData[i].second; currentType = ActionType::AnalogTrigger; break; }
            }
            if(targetIdx >= 0) break;
            for(int i = setEntry.stickStart; i < setEntry.stickEnd; i++){
                if(mActionSetData[i].first == actionName) { targetIdx = mActionSetData[i].second; currentType = ActionType::StickPadGyro; break; }
            }
            if(targetIdx >= 0) break;
            targetActionSet++;
        }
        //No action with that name was found.
        if(targetIdx < 0) return INVALID_ACTION_HANDLE;
        //The requested type was not the one found.
        if(currentType != type) return INVALID_ACTION_HANDLE;

        assert(targetIdx <= 255); //Right now I'm limiting it to eight bits.
        unsigned char passIdx = (unsigned char)targetIdx;
        const ActionHandleContents h = {type, passIdx, targetActionSet};
        ActionHandle handle = _produceActionHandle(h);

        return handle;
    }

    ActionHandle InputManager::getButtonActionHandle(const std::string& actionName){
        return _getActionHandle(ActionType::Button, actionName);
    }

    ActionHandle InputManager::getAxisActionHandle(const std::string& actionName){
        return _getActionHandle(ActionType::StickPadGyro, actionName);
    }

    ActionHandle InputManager::getAnalogTriggerActionHandle(const std::string& actionName){
        return _getActionHandle(ActionType::AnalogTrigger, actionName);
    }

    inline void InputManager::_printHandleError(const char* funcName) const{
        //TODO OPTIMISATION in future I'll cover this with a macro so it doesn't get considered in release builds.
        AV_ERROR("Action handle passed to {} is invalid.", funcName);
    }


    void InputManager::setButtonAction(InputDeviceId id, ActionHandle action, bool val){
        if(action == INVALID_ACTION_HANDLE) {
            _printHandleError("setButtonAction");
            return;
        }
        ActionHandleContents contents;
        _readActionHandle(&contents, action);

        assert(contents.type == ActionType::Button);

        mActionData[id].actionButtonData[contents.itemIdx] = val;
        mAnyDeviceData.actionButtonData[contents.itemIdx] += val ? 1 : -1;
    }

    bool InputManager::getButtonAction(InputDeviceId id, ActionHandle action) const{
        if(action == INVALID_ACTION_HANDLE){
            _printHandleError("getButtonAction");
            return false;
        }
        ActionHandleContents contents;
        _readActionHandle(&contents, action);

        assert(contents.type == ActionType::Button);
        if(id == ANY_INPUT_DEVICE){
            //This counts how many of that button were pressed, so if even one is pressed then return true.
            return mAnyDeviceData.actionButtonData[contents.itemIdx] > 0;
        }
        else if(id == KEYBOARD_INPUT_DEVICE){
            return mKeyboardData.actionButtonData[contents.itemIdx];
        }

        return mActionData[id].actionButtonData[contents.itemIdx];
    }

    float InputManager::getTriggerAction(InputDeviceId id, ActionHandle action) const{
        if(action == INVALID_ACTION_HANDLE){
            _printHandleError("getTriggerAction");
            return false;
        }
        ActionHandleContents contents;
        _readActionHandle(&contents, action);

        assert(contents.type == ActionType::AnalogTrigger);
        if(id == KEYBOARD_INPUT_DEVICE){
            return mKeyboardData.actionAnalogTriggerData[contents.itemIdx];
        }
        else if(id == ANY_INPUT_DEVICE){
            return mAnyDeviceData.actionAnalogTriggerData[contents.itemIdx];
        }

        return mActionData[id].actionAnalogTriggerData[contents.itemIdx];
    }

    float InputManager::getAxisAction(InputDeviceId id, ActionHandle action, bool x) const{
        if(action == INVALID_ACTION_HANDLE){
            _printHandleError("getAxisAction");
            return false;
        }
        ActionHandleContents contents;
        _readActionHandle(&contents, action);

        assert(contents.type == ActionType::StickPadGyro);
        const StickPadGyroData* target = 0;
        if(id == KEYBOARD_INPUT_DEVICE){
            target = &(mKeyboardData.actionStickPadGyroData[contents.itemIdx]);
        }
        else if(id == ANY_INPUT_DEVICE){
            target = &(mAnyDeviceData.actionStickPadGyroData[contents.itemIdx]);
        }
        else{
            target = &(mActionData[id].actionStickPadGyroData[contents.itemIdx]);
        }
        if(x) return target->x;
        return target->y;
    }

    void InputManager::setAxisAction(InputDeviceId id, ActionHandle action, bool x, float axis){
        assert(id >= 0 && id < MAX_INPUT_DEVICES);
        if(action == INVALID_ACTION_HANDLE){
            _printHandleError("setAxisAction");
            return;
        }
        ActionHandleContents contents;
        _readActionHandle(&contents, action);

        assert(contents.type == ActionType::StickPadGyro);

        //There's no need to check for the keyboard in these because the keyboard doesn't call them.
        StickPadGyroData* target = &(mActionData[id].actionStickPadGyroData[contents.itemIdx]);

        if(x){
            target->x = axis;
            mAnyDeviceData.actionStickPadGyroData[contents.itemIdx].x = axis;
        }else{
            target->y = axis;
            mAnyDeviceData.actionStickPadGyroData[contents.itemIdx].y = axis;
        }
    }

    void InputManager::setKeyboardKeyAction(ActionHandle action, float value){
        if(action == INVALID_ACTION_HANDLE){
            _printHandleError("setKeyboardKeyAction");
            return;
        }
        ActionHandleContents contents;
        _readActionHandle(&contents, action);

        if(contents.type == ActionType::StickPadGyro){
            //In this case read which axis to target from the handle.
            int targetAxis = _getHandleAxis(action);
            StickPadGyroData& target = mKeyboardData.actionStickPadGyroData[contents.itemIdx];
            StickPadGyroData& targetAny = mAnyDeviceData.actionStickPadGyroData[contents.itemIdx];

            if(targetAxis == 0){
                target.x = value;
                targetAny.x = value;
            }else if(targetAxis == 1){
                target.y = value;
                targetAny.y = value;
            }else if(targetAxis == 2){
                target.x = -value;
                targetAny.x = -value;
            }else if(targetAxis == 3){
                target.y = -value;
                targetAny.y = -value;
            }

        }else if(contents.type == ActionType::Button){
            bool pressed = value > 0 ? true : false;
            mKeyboardData.actionButtonData[contents.itemIdx] = pressed;
            mAnyDeviceData.actionButtonData[contents.itemIdx] += pressed ? 1 : -1;
        }else if(contents.type == ActionType::AnalogTrigger){
            mKeyboardData.actionAnalogTriggerData[contents.itemIdx] = value;
            mAnyDeviceData.actionAnalogTriggerData[contents.itemIdx] = value;
        }
    }

    void InputManager::setAnalogTriggerAction(InputDeviceId id, ActionHandle action, float axis){
        if(action == INVALID_ACTION_HANDLE){
            _printHandleError("setAnalogTriggerAction");
            return;
        }
        ActionHandleContents contents;
        _readActionHandle(&contents, action);

        assert(contents.type == ActionType::AnalogTrigger);
        assert(contents.itemIdx < mActionData[id].actionAnalogTriggerData.size());
        mActionData[id].actionAnalogTriggerData[contents.itemIdx] = axis;
        mAnyDeviceData.actionAnalogTriggerData[contents.itemIdx] = axis;
    }

    int InputManager::_getHandleAxis(ActionHandle handle){
        int outIdx = ((handle & 0x38000000) >> 27);
        assert(outIdx >= 0 && outIdx < 4);
        return outIdx;
    }

    void InputManager::_readActionHandle(ActionHandleContents* outContents, ActionHandle handle) const{
        outContents->type = (ActionType)((handle & 0xC0000000) >> 30);
        outContents->actionSetId = (unsigned char)((handle & 0xFF00) >> 8);
        outContents->itemIdx = (unsigned char)((handle & 0xFF));
    }

    ActionHandle InputManager::_produceActionHandle(const ActionHandleContents& contents) const{
        ActionHandle outHandle = 0x0;

        //Should have max 4
        outHandle |= ((unsigned char)contents.type << 30);

        outHandle |= (unsigned int)(contents.actionSetId) << 8;
        outHandle |= (unsigned int)contents.itemIdx;

        return outHandle;
    }

    inline void InputManager::_resetDeviceData(InputDeviceData& d) const{
        d.populated = false;
        d.deviceName[0] = '\0';
    }

    const char* InputManager::getDeviceName(InputDeviceId id) const {
        assert(id >= 0 && id < MAX_INPUT_DEVICES);
        return mDevices[id].deviceName;
    }

    void InputManager::setMouseButton(int mouseButton, bool pressed){
        if(mouseButton < 0 || mouseButton >= NUM_MOUSE_BUTTONS) return;

        mMouseButtons[mouseButton] = pressed;
    }

    int InputManager::getMouseButton(int mouseButton) const{
        if(mouseButton < 0 || mouseButton >= NUM_MOUSE_BUTTONS) return 0;

        return mMouseButtons[mouseButton];
    }

}
