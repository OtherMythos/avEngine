#include "Input.h"

namespace AV{
    bool Input::keys[] = {};

    void Input::setKeyActive(Input_Key key, bool active){
        if(key < 0 || key > numKeys) return;

        keys[key] = active;
    }

    bool Input::getKey(Input_Key key){
        if(key < 0 || key > numKeys) return false;

        return keys[key];
    }
}
