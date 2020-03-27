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

    void SDL2InputMapper::initialise(InputManager* inMan){
        mInputManager = inMan;
    }

    void SDL2InputMapper::setActionSetForDevice(InputDeviceId device, ActionSetHandle id){
        mDeviceActionSets[device] = id;
    }

    void SDL2InputMapper::setNumActionSets(int num){
        mMap.clear();
        mMap.emplace_back(MappedData());
    }

    void SDL2InputMapper::setupMap(){
        //OPTIMISATION to improve startup I should have something to return handles when they're created, so they can just be passed here.
        //Otherwise this is an o(n) lookup for each.

        mMap.clear();
        int numActionSets = mInputManager->getNumActionSets();
        setNumActionSets(1);
        assert(numActionSets == 1); //Right now I'm assuming this as the default only has the one action set. This is the function to setup with the default.

        for(int y = 0; y < numActionSets; y++){
            for(int i = 0; i < MAX_AXIS; i++)
                mMap[y].mappedAxis[i] = INVALID_ACTION_HANDLE;
            for(int i = 0; i < MAX_BUTTONS; i++)
                mMap[y].mappedButtons[i] = INVALID_ACTION_HANDLE;
            for(int i = 0; i < MAX_KEYS; i++)
                mMap[y].mappedKeys[i] = INVALID_ACTION_HANDLE;
        }

        ActionHandle leftMove = mInputManager->getAxisActionHandle("LeftMove");
        ActionHandle rightMove = mInputManager->getAxisActionHandle("RightMove");
        mMap[0].mappedAxis[(int)SDL_CONTROLLER_AXIS_LEFTX] = leftMove;
        mMap[0].mappedAxis[(int)SDL_CONTROLLER_AXIS_LEFTY] = leftMove;
        mMap[0].mappedAxis[(int)SDL_CONTROLLER_AXIS_RIGHTX] = rightMove;
        mMap[0].mappedAxis[(int)SDL_CONTROLLER_AXIS_RIGHTY] = rightMove;

        mMap[0].mappedButtons[(int)SDL_CONTROLLER_BUTTON_A] = mInputManager->getButtonActionHandle("Accept");
        mMap[0].mappedButtons[(int)SDL_CONTROLLER_BUTTON_B] = mInputManager->getButtonActionHandle("Decline");
        mMap[0].mappedButtons[(int)SDL_CONTROLLER_BUTTON_X] = mInputManager->getButtonActionHandle("Menu");
        mMap[0].mappedButtons[(int)SDL_CONTROLLER_BUTTON_Y] = mInputManager->getButtonActionHandle("Options");
        mMap[0].mappedButtons[(int)SDL_CONTROLLER_BUTTON_START] = mInputManager->getButtonActionHandle("Start");
        mMap[0].mappedButtons[(int)SDL_CONTROLLER_BUTTON_BACK] = mInputManager->getButtonActionHandle("Select");
    }

    ActionHandle SDL2InputMapper::getAxisMap(InputDeviceId device, int axis){
        return mMap[mDeviceActionSets[device]].mappedAxis[axis];
    }

    ActionHandle SDL2InputMapper::getButtonMap(InputDeviceId device, int button){
        return mMap[mDeviceActionSets[device]].mappedButtons[button];
    }

    void SDL2InputMapper::mapControllerInput(int key, ActionHandle action){
        InputManager::ActionHandleContents contents;
        mInputManager->_readActionHandle(&contents, action);

        if(contents.type == ActionType::Button){
            if(key >= MAX_BUTTONS || key < 0) key = 0;
            mMap[contents.actionSetId].mappedButtons[key] = action;
        }else if(contents.type == ActionType::StickPadGyro){
            //1 Right Axis
            //Anything else Left axis
            mMap[contents.actionSetId].mappedAxis[key == 1 ? SDL_CONTROLLER_AXIS_RIGHTX : SDL_CONTROLLER_AXIS_LEFTX] = action;
            mMap[contents.actionSetId].mappedAxis[key == 1 ? SDL_CONTROLLER_AXIS_RIGHTY : SDL_CONTROLLER_AXIS_LEFTY] = action;
        }else if(contents.type == ActionType::AnalogTrigger){
            static const SDL_GameControllerAxis axises[2] = {
                SDL_CONTROLLER_AXIS_TRIGGERLEFT,
                SDL_CONTROLLER_AXIS_TRIGGERRIGHT
            };
            if(key > 1 || key < 0) key = 0;
            mMap[contents.actionSetId].mappedAxis[axises[key]] = action;
        }
    }
}
