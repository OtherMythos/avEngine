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
        memset(&mappedAxis, (int)GuiInputTypes::None, sizeof(mappedAxis));

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
            const std::map<int, ActionHandle>& targetMap = mMap[contents.actionSetId].mappedButtons;
            for(auto it = targetMap.begin(); it != targetMap.end(); ++it){
                if(it->second == action)
                    return it->second;
            }
        }
        else if(dev == KEYBOARD_INPUT_DEVICE){
            const std::map<int, ActionHandle>& targetMap = mMap[contents.actionSetId].mappedKeys;
            for(auto it = targetMap.begin(); it != targetMap.end(); ++it){
                if(it->second == action)
                    return it->second;
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
        mappedGuiKeys[(int)SDL_SCANCODE_UP] = GuiInputTypes::Top;
        mappedGuiKeys[(int)SDL_SCANCODE_DOWN] = GuiInputTypes::Bottom;
        mappedGuiKeys[(int)SDL_SCANCODE_LEFT] = GuiInputTypes::Left;
        mappedGuiKeys[(int)SDL_SCANCODE_RIGHT] = GuiInputTypes::Right;
        mappedGuiKeys[(int)SDL_SCANCODE_RETURN] = GuiInputTypes::Primary;

        mappedGuiButtons[(int)SDL_CONTROLLER_BUTTON_A] = GuiInputTypes::Primary;
        mappedGuiButtons[(int)SDL_CONTROLLER_BUTTON_DPAD_UP] = GuiInputTypes::Top;
        mappedGuiButtons[(int)SDL_CONTROLLER_BUTTON_DPAD_DOWN] = GuiInputTypes::Bottom;
        mappedGuiButtons[(int)SDL_CONTROLLER_BUTTON_DPAD_LEFT] = GuiInputTypes::Left;
        mappedGuiButtons[(int)SDL_CONTROLLER_BUTTON_DPAD_RIGHT] = GuiInputTypes::Right;

        //TODO this could probably be dropped to just the two values per struct.
        mappedAxis[SDL_CONTROLLER_AXIS_LEFTX] = {
            GuiInputTypes::Top,
            GuiInputTypes::Bottom,
            GuiInputTypes::Left,
            GuiInputTypes::Right
        };
        mappedAxis[SDL_CONTROLLER_AXIS_LEFTY] = {
            GuiInputTypes::Top,
            GuiInputTypes::Bottom,
            GuiInputTypes::Left,
            GuiInputTypes::Right
        };
    }

    ActionHandle SDL2InputMapper::getAxisMap(InputDeviceId device, int axis){
        assert(axis < MAX_AXIS);
        MappedData& mapData = mMap[mDeviceActionSets[device]];
        auto it = mapData.mappedAxis.find(axis);
        return it == mapData.mappedAxis.end() ? INVALID_ACTION_HANDLE : it->second;
    }

    ActionHandle SDL2InputMapper::getButtonMap(InputDeviceId device, int button){
        //Assert here rather than if.
        //It's more efficient, and assuming nothing changes in SDL2 then there should be no case of a larger button being pressed.
        assert(button < MAX_BUTTONS);
        MappedData& mapData = mMap[mDeviceActionSets[device]];
        auto it = mapData.mappedButtons.find(button);
        return it == mapData.mappedButtons.end() ? INVALID_ACTION_HANDLE : it->second;
    }

    ActionHandle SDL2InputMapper::getKeyboardMap(int key){
        return mMap[mKeyboardActionSet].mappedKeys[key];
    }

    bool SDL2InputMapper::mapControllerInput(int key, ActionHandle action){
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
        return true;
    }

    bool SDL2InputMapper::mapKeyboardInput(int key, ActionHandle action){
        InputManager::ActionHandleContents contents;
        mInputManager->_readActionHandle(&contents, action);

        if(contents.type == ActionType::Button){
            mMap[contents.actionSetId].mappedKeys[key] = action;
        }else if(contents.type == ActionType::StickPadGyro){
            assert(false && "Axis actions should be mapped using the mapKeyboardAxis function.");
        }else if(contents.type == ActionType::AnalogTrigger){
            mMap[contents.actionSetId].mappedKeys[key] = action;
        }
        return true;
    }

    bool SDL2InputMapper::mapKeyboardAxis(int posX, int posY, int negX, int negY, ActionHandle action){
        InputManager::ActionHandleContents contents;
        mInputManager->_readActionHandle(&contents, action);
        assert(contents.type == ActionType::StickPadGyro);

        mMap[contents.actionSetId].mappedKeys[posX] = _wrapAxisTypeToHandle(action, 0);
        mMap[contents.actionSetId].mappedKeys[posY] = _wrapAxisTypeToHandle(action, 1);
        mMap[contents.actionSetId].mappedKeys[negX] = _wrapAxisTypeToHandle(action, 2);
        mMap[contents.actionSetId].mappedKeys[negY] = _wrapAxisTypeToHandle(action, 3);

        return true;
    }

    void SDL2InputMapper::clearAllMapping(){
        for(MappedData& d : mMap){
            d.mappedKeys.clear();
            d.mappedAxis.clear();
            d.mappedButtons.clear();
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

    void SDL2InputMapper::mapGuiControllerAxis(int axis, GuiInputTypes top, GuiInputTypes bottom, GuiInputTypes left, GuiInputTypes right){
        if(axis >= MAX_AXIS) return;

        mappedAxis[axis] = {top, bottom, left, right};
    }

    GuiInputTypes SDL2InputMapper::getGuiActionForKey(int key) const{
        if(key >= MAX_KEYS) return GuiInputTypes::None;
        return mappedGuiKeys[key];
    }

    GuiInputTypes SDL2InputMapper::getGuiActionForButton(int key) const{
        if(key >= MAX_BUTTONS) return GuiInputTypes::None;
        return mappedGuiButtons[key];
    }

    bool SDL2InputMapper::getGuiActionForAxis(int key, GuiMappedAxisData* outData) const{
        if(key >= MAX_AXIS) return false;

        //Assume if one is none they all are.
        if(mappedAxis[key].top == GuiInputTypes::None) return false;

        *outData = mappedAxis[key];
        return true;
    }
}
