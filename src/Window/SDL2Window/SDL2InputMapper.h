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
        ActionHandle getKeyboardMap(int key);

        //Setup a default controller mapping for SDL inputs.
        void setupMap();

        void setActionSetForDevice(InputDeviceId device, ActionSetHandle id);

        void mapControllerInput(int key, ActionHandle action);
        void mapKeyboardInput(int key, ActionHandle action);
        /**
        Mapping an axis presents a problem. With a keyboard you can map say four keys to each direction of the axis.
        However, how would you know at this point what action that key represented. So here I need to keep track of what that key represents somehow.
        So here I write some extra bits to the handle to represent the four axises. A normal handle would be missing these.
        Then, when interpreted by the input manager it can determine which direction of the axis to target.
        */
        void mapKeyboardAxis(int posX, int posY, int negX, int negY, ActionHandle action);

        /**
        Set the number of action sets for the mapper.
        This should be called when the action sets changes.
        */
        void setNumActionSets(int num);

        void clearAllMapping();

    private:
        //My own values for how many entries to expect might be a bit overblown.
        //That's not that big of a problem though. The space might be useful in the future (for instance I might consider the mouse to be an axis).
        static const int MAX_KEYS = 255;
        static const int MAX_AXIS = 12;
        static const int MAX_BUTTONS = 20;

        InputManager* mInputManager;

        //One of these will be created per action set.
        struct MappedData{
            //Here each index represents one of SDL's input types.
            //It will have an appropriate handle to point to.
            ActionHandle mappedKeys[MAX_KEYS];
            ActionHandle mappedAxis[MAX_AXIS];
            ActionHandle mappedButtons[MAX_BUTTONS];
        };

        //The current action set for each device.
        ActionSetHandle mDeviceActionSets[MAX_INPUT_DEVICES];
        //The keyboard has its own current action set.
        ActionSetHandle mKeyboardActionSet;
        //Each action set is expected to have an entry in here.
        std::vector<MappedData> mMap;

        inline bool _boundsCheckKey(int key) { return key < MAX_KEYS && key > 0; }
    };
}
