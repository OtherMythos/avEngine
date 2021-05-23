#include "SDL2InputMapper.h"

#include <SDL.h>
#include "Input/InputManager.h"
#include <string>
#include <cassert>
#include "Logger/Log.h"

#include "System/EnginePrerequisites.h"

namespace AV{
    SDL2InputMapper::SDL2InputMapper(){
        for(int i = 0; i < MAX_INPUT_DEVICES; i++)
            mDeviceActionSets[i] = 0;

        memset(&mappedGuiButtons, (int)GuiInputTypes::None, sizeof(mappedGuiButtons));
        memset(&mappedGuiKeys, (int)GuiInputTypes::None, sizeof(mappedGuiKeys));

        //Populate with a single entry just to avoid memory errors.
        setNumActionSets(1);
        mKeyboardActionSet = 0;
    }

    SDL2InputMapper::~SDL2InputMapper(){

    }

    void SDL2InputMapper::initialise(InputManager* inMan){
        mInputManager = inMan;
    }

    void SDL2InputMapper::setActionSetForDevice(InputDeviceId device, ActionSetHandle id){
        ActionSetHandle oldSet = INVALID_ACTION_SET_HANDLE;
        if(device == ANY_INPUT_DEVICE){
            mInputManager->notifyDeviceChangedActionSet(this, id, mKeyboardActionSet, KEYBOARD_INPUT_DEVICE);
            mKeyboardActionSet = id;
            for(int i = 0; i < MAX_INPUT_DEVICES; i++){
                mInputManager->notifyDeviceChangedActionSet(this, id, mDeviceActionSets[i], i);
                mDeviceActionSets[i] = id;
            }
            return;
        }
        if(device == KEYBOARD_INPUT_DEVICE){
            mInputManager->notifyDeviceChangedActionSet(this, id, mKeyboardActionSet, device);
            mKeyboardActionSet = id;
            return;
        }

        mInputManager->notifyDeviceChangedActionSet(this, id, mDeviceActionSets[device], device);
        mDeviceActionSets[device] = id;
    }

    ActionHandle SDL2InputMapper::isActionMappedToActionSet(InputDeviceId dev, ActionHandle action, ActionSetHandle targetSet) const{
        InputManager::ActionHandleContents contents;
        mInputManager->_readActionHandle(&contents, action);
        assert(contents.actionSetId != targetSet);

        static const uint32 INVALID = 3000;
        uint32 val = INVALID;
        if(dev < MAX_INPUT_DEVICES){
            for(uint32 i = 0; i < MAX_BUTTONS; i++){
                ActionHandle handle = mMap[contents.actionSetId].mappedButtons[i];
                if(handle == action){
                    return mMap[targetSet].mappedButtons[i];
                }
            }
        }
        else if(dev == KEYBOARD_INPUT_DEVICE){
            for(uint32 i = 0; i < MAX_KEYS; i++){
                ActionHandle handle = mMap[contents.actionSetId].mappedKeys[i];
                if(handle == action){
                    return mMap[targetSet].mappedKeys[i];
                }
            }
        }

        return INVALID_ACTION_HANDLE;
    }

    void SDL2InputMapper::setNumActionSets(int num){
        if(num != mMap.size()){
            mMap.clear();
            for(int i = 0; i < num; i++)
                mMap.emplace_back(MappedData());
        }
        clearAllMapping();
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

        mMap[0].mappedButtons[(int)SDL_CONTROLLER_BUTTON_DPAD_UP] = mInputManager->getButtonActionHandle("DirectionUp");
        mMap[0].mappedButtons[(int)SDL_CONTROLLER_BUTTON_DPAD_DOWN] = mInputManager->getButtonActionHandle("DirectionDown");
        mMap[0].mappedButtons[(int)SDL_CONTROLLER_BUTTON_DPAD_LEFT] = mInputManager->getButtonActionHandle("DirectionLeft");
        mMap[0].mappedButtons[(int)SDL_CONTROLLER_BUTTON_DPAD_RIGHT] = mInputManager->getButtonActionHandle("DirectionRight");

        mMap[0].mappedKeys[(int)SDLK_z] = accept;
        mMap[0].mappedKeys[(int)SDLK_x] = decline;
        mapKeyboardAxis((int)SDLK_d, (int)SDLK_s, (int)SDLK_a, (int)SDLK_w, leftMove);

        //Map the gui input
        mappedGuiKeys[(int)(SDLK_UP & ~SDLK_SCANCODE_MASK)] = GuiInputTypes::Top;
        mappedGuiKeys[(int)(SDLK_DOWN & ~SDLK_SCANCODE_MASK)] = GuiInputTypes::Bottom;
        mappedGuiKeys[(int)(SDLK_LEFT & ~SDLK_SCANCODE_MASK)] = GuiInputTypes::Left;
        mappedGuiKeys[(int)(SDLK_RIGHT & ~SDLK_SCANCODE_MASK)] = GuiInputTypes::Right;
        mappedGuiKeys[(int)(SDLK_RETURN & ~SDLK_SCANCODE_MASK)] = GuiInputTypes::Primary;

        mappedGuiButtons[(int)SDL_CONTROLLER_BUTTON_A] = GuiInputTypes::Primary;
        mappedGuiButtons[(int)SDL_CONTROLLER_BUTTON_DPAD_UP] = GuiInputTypes::Top;
        mappedGuiButtons[(int)SDL_CONTROLLER_BUTTON_DPAD_DOWN] = GuiInputTypes::Bottom;
        mappedGuiButtons[(int)SDL_CONTROLLER_BUTTON_DPAD_LEFT] = GuiInputTypes::Left;
        mappedGuiButtons[(int)SDL_CONTROLLER_BUTTON_DPAD_RIGHT] = GuiInputTypes::Right;
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

    void SDL2InputMapper::mapGuiControllerInput(int button, GuiInputTypes t){
        if(button >= MAX_BUTTONS || button < 0) return;
        mappedGuiButtons[button] = t;
    }

    void SDL2InputMapper::mapGuiKeyboardInput(int key, GuiInputTypes t){
        key = key & ~SDLK_SCANCODE_MASK;
        if(!_boundsCheckKey(key)) return;

        mappedGuiKeys[key] = t;
    }

    GuiInputTypes SDL2InputMapper::getGuiActionForKey(int key) const{
        if(key >= MAX_KEYS) return GuiInputTypes::None;
        return mappedGuiKeys[key];
    }

    GuiInputTypes SDL2InputMapper::getGuiActionForButton(int key) const{
        if(key >= MAX_BUTTONS) return GuiInputTypes::None;
        return mappedGuiButtons[key];
    }
}
