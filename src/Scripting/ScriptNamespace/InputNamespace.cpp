#include "InputNamespace.h"

#include "Input/Input.h"

namespace AV {
    SQInteger InputNamespace::getKey(HSQUIRRELVM vm){
        SQInteger key = 0;
        sq_getinteger(vm, -1, &key);
        
        SQBool result = Input::getKey((Input::Input_Key)key);
        
        sq_pushbool(vm, result);
        
        return 1;
    }
    
    void InputNamespace::setupNamespace(HSQUIRRELVM vm){
        _addFunction(vm, getKey, "getKey", 2, ".i");
    }
    
    void InputNamespace::setupConstants(HSQUIRRELVM vm){
        //So I ran into some problems with the const table.
        //For some reason I was unable to assign things in the const table and have that work.
        //It's more than likely something strange with my callback setup.
        //That should be investigated at some point in the future, but it's good enough for now.
        sq_pushroottable(vm);
        
        _declareConstant(vm, "_KeyNull", Input::Key_Null);
        _declareConstant(vm, "_KeyAccept", Input::Key_Accept);
        _declareConstant(vm, "_KeyDecline", Input::Key_Decline);
        _declareConstant(vm, "_KeyUp", Input::Key_Up);
        _declareConstant(vm, "_KeyDown", Input::Key_Down);
        _declareConstant(vm, "_KeyLeft", Input::Key_Left);
        _declareConstant(vm, "_KeyRight", Input::Key_Right);
        _declareConstant(vm, "_KeyDeveloperGuiToggle", Input::Key_DeveloperGuiToggle);
        
        sq_pop(vm, 1); //pop the const table
    }
}
