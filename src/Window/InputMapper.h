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
        Is an action handle mapped to an input in a certain action set.
        @returns If mapped the handle to that action in the target set.
        */
        virtual ActionHandle isActionMappedToActionSet(InputDeviceId dev, ActionHandle action, ActionSetHandle targetSet) const = 0;

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
        Map keyboard input to an axis.
        Four keys must be supplied, as well as an axis action.
        */
        virtual void mapKeyboardAxis(int posX, int posY, int negX, int negY, ActionHandle action) = 0;

        /**
        Map a controller input to a gui type.
        */
        virtual void mapGuiControllerInput(int button, GuiInputTypes t) = 0;
        /**
        Map a keyboard input to a gui type.
        */
        virtual void mapGuiKeyboardInput(int key, GuiInputTypes t) = 0;

        virtual GuiInputTypes getGuiActionForKey(int key) const = 0;
        virtual GuiInputTypes getGuiActionForButton(int key) const = 0;

        /**
        Clear all mappings by setting all values to the invalid handle option.
        */
        virtual void clearAllMapping() = 0;

        /**
        Set the number of action sets this mapper should list.
        This would be called by some procedure that sets up action sets.
        */
        virtual void setNumActionSets(int num) = 0;

        static ActionHandle _wrapAxisTypeToHandle(ActionHandle handle, int axis);
    };
}
