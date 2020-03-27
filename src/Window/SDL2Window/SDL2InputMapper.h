#pragma once

#include "Window/InputMapper.h"
#include <vector>

namespace AV{
    class InputManager;

    /**
    Maps inputs for SDL2.
    */
    class SDL2InputMapper : public InputMapper{
    public:
        SDL2InputMapper();
        ~SDL2InputMapper();

        void initialise(InputManager* inMan);

        //Axis accounts for both sticks and triggers. Triggers are provided to sdl as an axis.
        ActionHandle getAxisMap(InputDeviceId device, int axis);
        ActionHandle getButtonMap(InputDeviceId device, int axis);

        //Setup a default controller mapping for SDL inputs.
        void setupMap();

        void setActionSetForDevice(InputDeviceId device, ActionSetHandle id);

        void mapControllerInput(int key, ActionHandle action);

        /**
        Set the number of action sets for the mapper.
        This should be called when the action sets changes.
        */
        void setNumActionSets(int num);

    private:
        //My own values for how many entries to expect might be a bit overblown.
        //That's not that big of a problem though. The space might be useful in the future (for instance I might consider the mouse to be an axis).
        static const int MAX_KEYS = 255;
        static const int MAX_AXIS = 12;
        static const int MAX_BUTTONS = 20;

        InputManager* mInputManager;

        struct MappedData{
            //Here each index represents one of SDL's input types.
            //It will have an appropriate handle to point to.
            ActionHandle mappedKeys[MAX_KEYS];
            ActionHandle mappedAxis[MAX_AXIS];
            ActionHandle mappedButtons[MAX_BUTTONS];
        };

        //The current action set for each device.
        ActionSetHandle mDeviceActionSets[MAX_INPUT_DEVICES];
        //Each action set is expected to have an entry in here.
        std::vector<MappedData> mMap;
    };
}
