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

    SQInteger InputNamespace::getMouseX(HSQUIRRELVM vm){
        sq_pushinteger(vm, Input::getMouseX());

        return 1;
    }

    SQInteger InputNamespace::getMouseY(HSQUIRRELVM vm){
        sq_pushinteger(vm, Input::getMouseY());

        return 1;
    }

    SQInteger InputNamespace::getMouseButton(HSQUIRRELVM vm){
        SQInteger mouseButton = 0;
        sq_getinteger(vm, -1, &mouseButton);

        bool result = Input::getMouseButton(mouseButton);

        sq_pushbool(vm, result);
        return 1;
    }

    void InputNamespace::setupNamespace(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, getKey, "getKey", 2, ".i");
        ScriptUtils::addFunction(vm, getMouseX, "getMouseX");
        ScriptUtils::addFunction(vm, getMouseY, "getMouseY");
        ScriptUtils::addFunction(vm, getMouseButton, "getMouseButton", 2, ".i");
    }

    void InputNamespace::setupConstants(HSQUIRRELVM vm){
        //So I ran into some problems with the const table.
        //For some reason I was unable to assign things in the const table and have that work.
        //It's more than likely something strange with my callback setup.
        //That should be investigated at some point in the future, but it's good enough for now.
        sq_pushroottable(vm);

        ScriptUtils::declareConstant(vm, "_KeyNull", Input::Key_Null);
        ScriptUtils::declareConstant(vm, "_KeyAccept", Input::Key_Accept);
        ScriptUtils::declareConstant(vm, "_KeyDecline", Input::Key_Decline);
        ScriptUtils::declareConstant(vm, "_KeyUp", Input::Key_Up);
        ScriptUtils::declareConstant(vm, "_KeyDown", Input::Key_Down);
        ScriptUtils::declareConstant(vm, "_KeyLeft", Input::Key_Left);
        ScriptUtils::declareConstant(vm, "_KeyRight", Input::Key_Right);
        ScriptUtils::declareConstant(vm, "_KeyDeveloperGuiToggle", Input::Key_DeveloperGuiToggle);

        ScriptUtils::declareConstant(vm, "_MouseButtonLeft", 0);
        ScriptUtils::declareConstant(vm, "_MouseButtonRight", 1);

        sq_pop(vm, 1); //pop the const table
    }
}
