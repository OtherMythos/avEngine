#pragma once

#include "Window/InputMapper.h"

namespace AV{
    class InputManager;

    class iosInputMapper : public InputMapper{
    public:
        iosInputMapper();
        ~iosInputMapper();

       void initialise(InputManager* inMan);

        ActionHandle getAxisMap(InputDeviceId device, int axis);
        ActionHandle getButtonMap(InputDeviceId device, int axis);
        ActionHandle getKeyboardMap(int key);

        void setupMap();

        void setActionSetForDevice(InputDeviceId device, ActionSetHandle id);

        ActionHandle isActionMappedToActionSet(InputDeviceId dev, ActionHandle action, ActionSetHandle targetSet) const;

        void mapGuiControllerInput(int button, GuiInputTypes t);
        void mapGuiKeyboardInput(int key, GuiInputTypes t);
        void mapGuiControllerAxis(int axis, GuiInputTypes top, GuiInputTypes bottom, GuiInputTypes left, GuiInputTypes right);

        GuiInputTypes getGuiActionForKey(int key) const;
        GuiInputTypes getGuiActionForButton(int key) const;
        bool getGuiActionForAxis(int key, GuiMappedAxisData* outData) const;

        void mapControllerInput(int key, ActionHandle action);
        void mapKeyboardInput(int key, ActionHandle action);
        void mapKeyboardAxis(int posX, int posY, int negX, int negY, ActionHandle action);
        void setNumActionSets(int num);

        void clearAllMapping();

    private:
        InputManager* mInputManager;
    };

}
