#include "Input.h"

namespace AV{
    bool Input::keys[] = {};

    int Input::mouseX = 0;
    int Input::mouseY = 0;
    int Input::mouseWheel = 0;
    bool Input::mouseButtons[numMouseButtons] = {false, false, false};

    void Input::setKeyActive(Input_Key key, bool active){
        if(key < 0 || key > numKeys) return;

        keys[key] = active;
    }

    bool Input::getKey(Input_Key key){
        if(key < 0 || key > numKeys) return false;

        return keys[key];
    }

    void Input::setMouseButton(int mouseButton, bool pressed){
        if(mouseButton < 0 || mouseButton >= numMouseButtons) return;

        mouseButtons[mouseButton] = pressed;
    }

    int Input::getMouseButton(int mouseButton){
        if(mouseButton < 0 || mouseButton >= numMouseButtons) return 0;

        return mouseButtons[mouseButton];
    }
}
