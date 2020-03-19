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
    }

    InputManager::~InputManager(){

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
