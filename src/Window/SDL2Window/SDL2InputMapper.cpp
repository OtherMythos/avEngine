#include "SDL2InputMapper.h"

#include <SDL.h>
#include "Input/InputManager.h"
#include <string>
#include <cassert>

namespace AV{
    SDL2InputMapper::SDL2InputMapper(){
        for(int i = 0; i < MAX_INPUT_DEVICES; i++)
            mDeviceActionSets[i] = 0;
    }

    SDL2InputMapper::~SDL2InputMapper(){

    }

    void SDL2InputMapper::setActionSetForDevice(InputDeviceId device, ActionSetHandle id){
        mDeviceActionSets[device] = id;
    }

    void SDL2InputMapper::setupMap(InputManager* inMan){
        //OPTIMISATION to improve startup I should have something to return handles when they're created, so they can just be passed here.
        //Otherwise this is an o(n) lookup for each.

        mMap.clear();
        int numActionSets = inMan->getNumActionSets();
        mMap.reserve(numActionSets);
        assert(numActionSets == 1); //Right now I'm assuming this as the default only has the one action set. This is the function to setup with the default.

        for(int y = 0; y < numActionSets; y++){
            for(int i = 0; i < MAX_AXIS; i++)
                mMap[y].mappedAxis[i] = INVALID_ACTION_HANDLE;
            for(int i = 0; i < MAX_BUTTONS; i++)
                mMap[y].mappedButtons[i] = INVALID_ACTION_HANDLE;
            for(int i = 0; i < MAX_KEYS; i++)
                mMap[y].mappedKeys[i] = INVALID_ACTION_HANDLE;
        }

        ActionHandle leftMove = inMan->getAxisActionHandle("LeftMove");
        ActionHandle rightMove = inMan->getAxisActionHandle("RightMove");
        mMap[0].mappedAxis[(int)SDL_CONTROLLER_AXIS_LEFTX] = leftMove;
        mMap[0].mappedAxis[(int)SDL_CONTROLLER_AXIS_LEFTY] = leftMove;
        mMap[0].mappedAxis[(int)SDL_CONTROLLER_AXIS_RIGHTX] = rightMove;
        mMap[0].mappedAxis[(int)SDL_CONTROLLER_AXIS_RIGHTY] = rightMove;

        mMap[0].mappedButtons[(int)SDL_CONTROLLER_BUTTON_A] = inMan->getButtonActionHandle("Accept");
        mMap[0].mappedButtons[(int)SDL_CONTROLLER_BUTTON_B] = inMan->getButtonActionHandle("Decline");
        mMap[0].mappedButtons[(int)SDL_CONTROLLER_BUTTON_X] = inMan->getButtonActionHandle("Menu");
        mMap[0].mappedButtons[(int)SDL_CONTROLLER_BUTTON_Y] = inMan->getButtonActionHandle("Options");
        mMap[0].mappedButtons[(int)SDL_CONTROLLER_BUTTON_START] = inMan->getButtonActionHandle("Start");
        mMap[0].mappedButtons[(int)SDL_CONTROLLER_BUTTON_BACK] = inMan->getButtonActionHandle("Select");
    }

    ActionHandle SDL2InputMapper::getAxisMap(InputDeviceId device, int axis){
        return mMap[mDeviceActionSets[device]].mappedAxis[axis];
    }

    ActionHandle SDL2InputMapper::getButtonMap(InputDeviceId device, int button){
        return mMap[mDeviceActionSets[device]].mappedButtons[button];
    }
}
