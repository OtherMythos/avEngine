#include "InputManager.h"

#include <string.h>
#include "Logger/Log.h"

namespace AV{
    InputManager::InputManager(){
        for(int i = 0; i < MAX_INPUT_DEVICES; i++){
            _resetDeviceData(mDevices[i]);
        }

        //Populate the default action set. Temporary code.
        mActionSetMeta = { {"default", 0} };
        mActionSets = { {0, 1} };
        mActionSetData = {
            {"Move", 0}
        };
        mActionButtonData.push_back(false);
    }

    InputManager::~InputManager(){

    }

    ActionSetHandle InputManager::createActionSet(const char* actionSetName){
        size_t size = mActionSets.size();
        assert(mActionSets.size() <= 255 && "No more than 255 action sets can be created.");
        ActionSetHandle handle = (ActionSetHandle)size;

        mActionSetMeta[actionSetName] = handle;
        mActionSets.push_back({0, 0}); //The values will be populated later.

        return handle;
    }

    void InputManager::createAction(const char* actionName, ActionSetHandle actionSet, ActionType type){
        //TODO here I would do a check based on type of which list (digital button, axis, trigger) to model the index off of.
        mActionSetData.push_back({actionName, mActionButtonData.size()});
        mActionButtonData.push_back(false); //To increase the size.
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

        return targetId;
    }

    bool InputManager::removeInputDevice(InputDeviceId dev){
        if(dev >= MAX_INPUT_DEVICES) return false;
        InputDeviceData& d = mDevices[dev];

        if(!d.populated) return false;
        AV_INFO("Removed controller with id {}, named '{}'", dev, d.deviceName);

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

    ActionHandle InputManager::getDigitalActionHandle(const std::string& actionName){
        //TODO implement a search through this list.
        //Finds the correct string, creates the handle and returns it.

        int targetIdx = -1;
        for(int i = 0; i < mActionSetData.size(); i++){
            if(mActionSetData[i].first == actionName){
                //The value was found.
                targetIdx = i;
                break;
            }
        }
        //No action was found with that name.
        if(targetIdx < 0) return INVALID_ACTION_HANDLE;

        assert(targetIdx <= 255); //Right now I'm limiting it to eight bits.
        unsigned char passIdx = (unsigned char)targetIdx;
        const ActionHandleContents h = {ActionType::Button, passIdx, 0}; //TODO populate with the set value.
        ActionHandle handle = _produceActionHandle(h);

        return handle;
    }


    void InputManager::setDigitalAction(InputDeviceId id, ActionHandle action, bool val){
        ActionHandleContents contents;
        _readActionHandle(&contents, action);

        assert(contents.type == ActionType::Button);
        mActionButtonData[contents.itemIdx] = val;
    }

    bool InputManager::getDigitalAction(InputDeviceId id, ActionHandle action) const{
        ActionHandleContents contents;
        _readActionHandle(&contents, action);

        assert(contents.type == ActionType::Button);
        return mActionButtonData[contents.itemIdx];
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

    /**
    The first value is the value used to id things.
    The second is the localised string.

    In steam the user maps the localised string via the gui.
        The user maps the localised string via the gui.
    Otherwise the engine maps the inputs based on what it thinks things should be.


    actionSetFirst{
        StickPadGyro{
            "Move": "#Action_Move"
            "Camera": "#Action_Camera"
        }
        Buttons{
            "Jump": "#Action_Jump"
        }
    }
    menuActionSet{
        StickPadGyro{
        }
        Buttons{
            "Menu_Up": "#Menu_Up"
        }
    }

    default{
        StickPadGyro{
            "move": "#Action_Move"
        }
    }
    */
}
