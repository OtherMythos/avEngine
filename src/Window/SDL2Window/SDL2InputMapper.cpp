#include "SDL2InputMapper.h"

#include <SDL.h>
#include "Input/InputManager.h"
#include <string>
#include <cassert>
#include "Logger/Log.h"

namespace AV{
    SDL2InputMapper::SDL2InputMapper(){
        for(int i = 0; i < MAX_INPUT_DEVICES; i++)
            mDeviceActionSets[i] = 0;

        mKeyboardActionSet = 0;
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
        setNumActionSets(1); //Right now I'm assuming this as the default only has the one action set. This is the function to setup with the default.

        clearAllMapping();

        ActionHandle leftMove = mInputManager->getAxisActionHandle("LeftMove");
        ActionHandle rightMove = mInputManager->getAxisActionHandle("RightMove");
        mMap[0].mappedAxis[(int)SDL_CONTROLLER_AXIS_LEFTX] = leftMove;
        mMap[0].mappedAxis[(int)SDL_CONTROLLER_AXIS_LEFTY] = leftMove;
        mMap[0].mappedAxis[(int)SDL_CONTROLLER_AXIS_RIGHTX] = rightMove;
        mMap[0].mappedAxis[(int)SDL_CONTROLLER_AXIS_RIGHTY] = rightMove;

        ActionHandle accept = mInputManager->getButtonActionHandle("Accept");
        ActionHandle decline = mInputManager->getButtonActionHandle("Decline");
        mMap[0].mappedButtons[(int)SDL_CONTROLLER_BUTTON_A] = accept;
        mMap[0].mappedButtons[(int)SDL_CONTROLLER_BUTTON_B] = decline;
        mMap[0].mappedButtons[(int)SDL_CONTROLLER_BUTTON_X] = mInputManager->getButtonActionHandle("Menu");
        mMap[0].mappedButtons[(int)SDL_CONTROLLER_BUTTON_Y] = mInputManager->getButtonActionHandle("Options");
        mMap[0].mappedButtons[(int)SDL_CONTROLLER_BUTTON_START] = mInputManager->getButtonActionHandle("Start");
        mMap[0].mappedButtons[(int)SDL_CONTROLLER_BUTTON_BACK] = mInputManager->getButtonActionHandle("Select");

        mMap[0].mappedKeys[(int)SDLK_z] = accept;
        mMap[0].mappedKeys[(int)SDLK_x] = decline;
    }

    ActionHandle SDL2InputMapper::getAxisMap(InputDeviceId device, int axis){
        return mMap[mDeviceActionSets[device]].mappedAxis[axis];
    }

    ActionHandle SDL2InputMapper::getButtonMap(InputDeviceId device, int button){
        return mMap[mDeviceActionSets[device]].mappedButtons[button];
    }

    ActionHandle SDL2InputMapper::getKeyboardMap(int key){
        if(key >= MAX_KEYS) return INVALID_ACTION_HANDLE;
        return mMap[mKeyboardActionSet].mappedKeys[key];
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

    void SDL2InputMapper::mapKeyboardInput(int key, ActionHandle action){
        if(!_boundsCheckKey(key)) return;

        InputManager::ActionHandleContents contents;
        mInputManager->_readActionHandle(&contents, action);

        if(contents.type == ActionType::Button){
            mMap[contents.actionSetId].mappedKeys[key] = action;
        }else if(contents.type == ActionType::StickPadGyro){
            assert(false && "Axis actions should be mapped using the mapKeyboardAxis function.");
        }else if(contents.type == ActionType::AnalogTrigger){
            mMap[contents.actionSetId].mappedKeys[key] = action;
        }
    }

    void SDL2InputMapper::mapKeyboardAxis(int posX, int posY, int negX, int negY, ActionHandle action){
        //One of the provided axises is invalid.
        if( !(_boundsCheckKey(posX) && _boundsCheckKey(posY) && _boundsCheckKey(negX) && _boundsCheckKey(negY)) ) return;
        InputManager::ActionHandleContents contents;
        mInputManager->_readActionHandle(&contents, action);
        assert(contents.type == ActionType::StickPadGyro);

        mMap[contents.actionSetId].mappedKeys[posX] = _wrapAxisTypeToHandle(action, 0);
        mMap[contents.actionSetId].mappedKeys[posY] = _wrapAxisTypeToHandle(action, 1);
        mMap[contents.actionSetId].mappedKeys[negX] = _wrapAxisTypeToHandle(action, 2);
        mMap[contents.actionSetId].mappedKeys[negY] = _wrapAxisTypeToHandle(action, 3);
    }

    void SDL2InputMapper::clearAllMapping(){
        for(MappedData& d : mMap){
            for(int i = 0; i < MAX_AXIS; i++)
                d.mappedAxis[i] = INVALID_ACTION_HANDLE;
            for(int i = 0; i < MAX_BUTTONS; i++)
                d.mappedButtons[i] = INVALID_ACTION_HANDLE;
            for(int i = 0; i < MAX_KEYS; i++)
                d.mappedKeys[i] = INVALID_ACTION_HANDLE;
        }
    }
}
