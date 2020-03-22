#pragma once

#include "Window/InputMapper.h"

namespace AV{
    class InputManager;

    /**
    Maps inputs for SDL2.
    */
    class SDL2InputMapper : public InputMapper{
    public:
        SDL2InputMapper();
        ~SDL2InputMapper();

        ActionHandle getAxisMap(int axis);
        ActionHandle getButtonMap(int axis);

        //Setup a default controller mapping for SDL inputs.
        void setupMap(InputManager* inMan);

    private:
        //My own values for how many entries to expect might be a bit overblown.
        //That's not that big of a problem though. The space might be useful in the future (for instance I might consider the mouse to be an axis).
        static const int MAX_KEYS = 255;
        static const int MAX_AXIS = 12;
        static const int MAX_BUTTONS = 20;
        //Here each index represents one of SDL's input types.
        //It will have an appropriate handle to point to.
        ActionHandle mappedKeys[MAX_KEYS];
        ActionHandle mappedAxis[MAX_AXIS];
        ActionHandle mappedButtons[MAX_BUTTONS];
    };
}
