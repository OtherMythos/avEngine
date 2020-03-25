#include "InputManager.h"

#include <string.h>
#include "Logger/Log.h"

namespace AV{
    InputManager::InputManager(){
        for(int i = 0; i < MAX_INPUT_DEVICES; i++){
            _resetDeviceData(mDevices[i]);
        }

        //Populate the default action set. Temporary code.
        /*mActionSetMeta = { {"default", 0} };
        mActionSets = { {0, 1} };
        mActionSetData = {
            {"Move", 0}
        };
        mActionButtonData.push_back(false);*/
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
    }

    ActionSetHandle InputManager::createActionSet(const char* actionSetName){
        size_t size = mActionSets.size();
        assert(mActionSets.size() <= 255 && "No more than 255 action sets can be created.");
        ActionSetHandle handle = (ActionSetHandle)size;

        mActionSetMeta[actionSetName] = handle;
        mActionSets.push_back({0, 0}); //The values will be populated later.

        return handle;
    }

    void InputManager::clearAllActionSets(){
        mActionSets.clear();
        mActionSetData.clear();
        mActionSetMeta.clear();

        for(int i = 0; i < MAX_INPUT_DEVICES; i++){
            mActionData[i].actionButtonData.clear();
            mActionData[i].actionAnalogTriggerData.clear();
            mActionData[i].actionStickPadGyroData.clear();
        }
    }

    size_t InputManager::createAction(const char* actionName, ActionSetHandle actionSet, ActionType type, bool firstValue){
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
                infoStart = &e.stickStart;
                infoEnd = &e.stickEnd;
                break;
            }
            case ActionType::AnalogTrigger:{
                targetListSize = mActionData[0].actionAnalogTriggerData.size();
                for(int i = 0; i < MAX_INPUT_DEVICES; i++)
                    mActionData[i].actionAnalogTriggerData.push_back(0.0f);
                infoStart = &e.analogTriggerStart;
                infoEnd = &e.analogTriggerEnd;
                break;
            }
            case ActionType::Button:{
                targetListSize = mActionData[0].actionButtonData.size();
                for(int i = 0; i < MAX_INPUT_DEVICES; i++)
                    mActionData[i].actionButtonData.push_back(false);
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

        return targetId;
    }

    bool InputManager::removeInputDevice(InputDeviceId dev){
        if(dev >= MAX_INPUT_DEVICES) return false;
        InputDeviceData& d = mDevices[dev];

        if(!d.populated) return false;
        AV_INFO("Removed controller with id {}, named '{}'", dev, d.deviceName);
        mNumActiveControllers--;

        _resetDeviceData(d);

        return true;
    }

    void InputManager::setCurrentActionSet(ActionSetHandle actionSet){
        mCurrentActionSet = actionSet;
    }

    ActionSetHandle InputManager::getActionSetHandle(const std::string& setName) const{
        const auto& it = mActionSetMeta.find(setName);
        if(it == mActionSetMeta.end()) return INVALID_ACTION_SET_HANDLE;

        return (*it).second;
    }

    ActionHandle InputManager::_getActionHandle(ActionType type, const std::string& actionName){
        //TODO IMPORTANT
        //Right now I search the action set values, when really I should be having to search each set to search the individual types.
        //I have to take it on good faith that the requested action types do indeed match (which they might not).
        int targetIdx = -1;
        for(int i = 0; i < mActionSetData.size(); i++){
            if(mActionSetData[i].first == actionName){
                //The value was found.
                //targetIdx = i;
                targetIdx = mActionSetData[i].second;
                break;
            }
        }
        //No action was found with that name.
        if(targetIdx < 0) return INVALID_ACTION_HANDLE;

        assert(targetIdx <= 255); //Right now I'm limiting it to eight bits.
        unsigned char passIdx = (unsigned char)targetIdx;
        const ActionHandleContents h = {type, passIdx, 0}; //TODO populate with the set value.
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
    }

    bool InputManager::getButtonAction(InputDeviceId id, ActionHandle action) const{
        if(action == INVALID_ACTION_HANDLE){
            _printHandleError("getButtonAction");
            return false;
        }
        ActionHandleContents contents;
        _readActionHandle(&contents, action);

        assert(contents.type == ActionType::Button);
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
        return mActionData[id].actionAnalogTriggerData[contents.itemIdx];
    }

    float InputManager::getAxisAction(InputDeviceId id, ActionHandle action, bool x) const{
        if(action == INVALID_ACTION_HANDLE){
            _printHandleError("getTriggerAction");
            return false;
        }
        ActionHandleContents contents;
        _readActionHandle(&contents, action);

        assert(contents.type == ActionType::StickPadGyro);
        const StickPadGyroData& d = mActionData[id].actionStickPadGyroData[contents.itemIdx];
        if(x) return d.x;
        return d.y;
    }

    void InputManager::setAxisAction(InputDeviceId id, ActionHandle action, bool x, float axis){
        if(action == INVALID_ACTION_HANDLE){
            _printHandleError("setAxisAction");
            return;
        }
        ActionHandleContents contents;
        _readActionHandle(&contents, action);

        assert(contents.type == ActionType::StickPadGyro);
        StickPadGyroData& target = mActionData[id].actionStickPadGyroData[contents.itemIdx];
        if(x) target.x = axis;
        else target.y = axis;
    }

    void InputManager::setAnalogTriggerAction(InputDeviceId id, ActionHandle action, float axis){
        if(action == INVALID_ACTION_HANDLE){
            _printHandleError("setAnalogTriggerAction");
            return;
        }
        ActionHandleContents contents;
        _readActionHandle(&contents, action);

        assert(contents.type == ActionType::AnalogTrigger);
        mActionData[id].actionAnalogTriggerData[contents.itemIdx] = axis;
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
        d.buttonPressed = false;
        d.deviceName[0] = '\0';
    }

    const char* InputManager::getDeviceName(InputDeviceId id) const {
        assert(id >= 0 && id < MAX_INPUT_DEVICES);
        return mDevices[id].deviceName;
    }
}
