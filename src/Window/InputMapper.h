#pragma once

#include "Input/InputPrerequisites.h"

namespace AV{

    /**
    A class responsible for mapping input types to actions.
    For instance, if an SDL window was in use, sdl keypresses/controller actions need to be mapped to actions.
    This class is responsible for managing action handles and their relevant keys.
    */
    class InputMapper{
    public:
        InputMapper();
        virtual ~InputMapper();

        /**
        Obtain the action handle for an axis movement.
        */
        virtual ActionHandle getAxisMap(InputDeviceId device, int axis) = 0;

        virtual ActionHandle getButtonMap(InputDeviceId device, int axis) = 0;

        virtual ActionHandle getKeyboardMap(int key) = 0;

        virtual void setActionSetForDevice(InputDeviceId device, ActionSetHandle id) = 0;

        /**
        Map a controller input to an action handle.
        The key will be interpreted according to the implementation.
        */
        virtual void mapControllerInput(int key, ActionHandle action) = 0;

        /**
        Map a keyboard key to an action handle.
        This mapping is quite dependant on what type of action is passed.
        */
        virtual void mapKeyboardInput(int key, ActionHandle action) = 0;

        /**
        Clear all mappings by setting all values to the invalid handle option.
        */
        virtual void clearAllMapping() = 0;
    };
}
