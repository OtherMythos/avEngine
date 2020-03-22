#include "SDL2InputMapper.h"

#include <SDL.h>
#include "Input/InputManager.h"

namespace AV{
    SDL2InputMapper::SDL2InputMapper(){
        for(int i = 0; i < MAX_AXIS; i++)
            mappedAxis[i] = INVALID_ACTION_HANDLE;
        for(int i = 0; i < MAX_BUTTONS; i++)
            mappedButtons[i] = INVALID_ACTION_HANDLE;
        for(int i = 0; i < MAX_KEYS; i++)
            mappedKeys[i] = INVALID_ACTION_HANDLE;
    }

    SDL2InputMapper::~SDL2InputMapper(){

    }

    void SDL2InputMapper::setupMap(InputManager* inMan){
        //OPTIMISATION to improve startup I should have something to return handles when they're created, so they can just be passed here.
        //Otherwise this is an o(n) lookup for each.

        ActionHandle leftMove = inMan->getAxisActionHandle("LeftMove");
        ActionHandle rightMove = inMan->getAxisActionHandle("RightMove");
        mappedAxis[(int)SDL_CONTROLLER_AXIS_LEFTX] = leftMove;
        mappedAxis[(int)SDL_CONTROLLER_AXIS_LEFTY] = leftMove;
        mappedAxis[(int)SDL_CONTROLLER_AXIS_RIGHTX] = rightMove;
        mappedAxis[(int)SDL_CONTROLLER_AXIS_RIGHTY] = rightMove;

        mappedButtons[(int)SDL_CONTROLLER_BUTTON_A] = inMan->getButtonActionHandle("Accept");
        mappedButtons[(int)SDL_CONTROLLER_BUTTON_B] = inMan->getButtonActionHandle("Decline");
        mappedButtons[(int)SDL_CONTROLLER_BUTTON_X] = inMan->getButtonActionHandle("Menu");
        mappedButtons[(int)SDL_CONTROLLER_BUTTON_Y] = inMan->getButtonActionHandle("Options");
        mappedButtons[(int)SDL_CONTROLLER_BUTTON_START] = inMan->getButtonActionHandle("Start");
        mappedButtons[(int)SDL_CONTROLLER_BUTTON_BACK] = inMan->getButtonActionHandle("Select");
    }

    ActionHandle SDL2InputMapper::getAxisMap(int axis){
        return mappedAxis[axis];
    }

    ActionHandle SDL2InputMapper::getButtonMap(int button){
        return mappedButtons[button];
    }
}
