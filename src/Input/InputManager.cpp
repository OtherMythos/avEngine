#include "InputManager.h"

#include <string.h>
#include "Logger/Log.h"

namespace AV{
    InputManager::InputManager(){
        for(int i = 0; i < MAX_INPUT_DEVICES; i++){
            _resetDeviceData(mDevices[i]);
        }
    }

    InputManager::~InputManager(){

    }

    bool InputManager::addInputDevice(InputDeviceId dev, const char* deviceName){
        if(dev >= MAX_INPUT_DEVICES) return false;
        InputDeviceData& d = mDevices[dev];

        if(d.populated) return false;

        strncpy(d.deviceName, deviceName, 20);
        d.populated = true;

        AV_INFO("Added a controller with id {}, as name '{}'", dev, deviceName);

        return true;
    }

    bool InputManager::removeInputDevice(InputDeviceId dev){
        if(dev >= MAX_INPUT_DEVICES) return false;
        InputDeviceData& d = mDevices[dev];

        if(!d.populated) return false;
        AV_INFO("Removed controller with id {}, named '{}'", dev, d.deviceName);

        _resetDeviceData(d);

        return true;
    }

    inline void InputManager::_resetDeviceData(InputDeviceData& d) const{
        d.populated = false;
        d.buttonPressed = false;
        d.deviceName[0] = '\0';
    }
}
