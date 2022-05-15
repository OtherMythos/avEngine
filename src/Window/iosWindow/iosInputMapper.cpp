#include "iosInputMapper.h"

namespace AV{
    iosInputMapper::iosInputMapper(){

    }

    iosInputMapper::~iosInputMapper(){

    }

    void iosInputMapper::initialise(InputManager* inMan){
        mInputManager = inMan;
    }

    void iosInputMapper::setActionSetForDevice(InputDeviceId device, ActionSetHandle id){

    }

    ActionHandle iosInputMapper::isActionMappedToActionSet(InputDeviceId dev, ActionHandle action, ActionSetHandle targetSet) const{
        return INVALID_ACTION_HANDLE;
    }

    void iosInputMapper::setNumActionSets(int num){

    }

    void iosInputMapper::setupMap(){

    }

    ActionHandle iosInputMapper::getAxisMap(InputDeviceId device, int axis){
        return INVALID_ACTION_HANDLE;
    }

    ActionHandle iosInputMapper::getButtonMap(InputDeviceId device, int button){
        return INVALID_ACTION_HANDLE;
    }

    ActionHandle iosInputMapper::getKeyboardMap(int key){
        return INVALID_ACTION_HANDLE;
    }

    void iosInputMapper::mapControllerInput(int key, ActionHandle action){

    }

    void iosInputMapper::mapKeyboardInput(int key, ActionHandle action){

    }

    void iosInputMapper::mapKeyboardAxis(int posX, int posY, int negX, int negY, ActionHandle action){

    }

    void iosInputMapper::clearAllMapping(){

    }

    void iosInputMapper::mapGuiControllerInput(int button, GuiInputTypes t){

    }

    void iosInputMapper::mapGuiKeyboardInput(int key, GuiInputTypes t){

    }

    void iosInputMapper::mapGuiControllerAxis(int axis, GuiInputTypes top, GuiInputTypes bottom, GuiInputTypes left, GuiInputTypes right){

    }

    GuiInputTypes iosInputMapper::getGuiActionForKey(int key) const{
        return GuiInputTypes::None;
    }

    GuiInputTypes iosInputMapper::getGuiActionForButton(int key) const{
        return GuiInputTypes::None;
    }

    bool iosInputMapper::getGuiActionForAxis(int key, GuiMappedAxisData* outData) const{
        return false;
    }


}
