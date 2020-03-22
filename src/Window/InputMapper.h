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
        virtual ActionHandle getAxisMap(int axis) = 0;

        virtual ActionHandle getButtonMap(int axis) = 0;
    };
}
