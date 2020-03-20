#include "InputNamespace.h"

#include "Input/Input.h"
#include "Input/InputManager.h"
#include "Scripting/ScriptObjectTypeTags.h"
#include "System/BaseSingleton.h"

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

    SQInteger InputNamespace::getDigitalActionHandle(HSQUIRRELVM vm){
        const SQChar *actionName;
        sq_getstring(vm, -1, &actionName);

        ActionHandle handle = BaseSingleton::getInputManager()->getDigitalActionHandle(actionName);
        if(handle == INVALID_ACTION_HANDLE) return sq_throwerror(vm, "Error retreiving action handle.");

        ActionHandle* pointer = (ActionHandle*)sq_newuserdata(vm, sizeof(ActionHandle));
        *pointer = handle;

        sq_settypetag(vm, -1, DigitalActionHandleTypeTag);

        return 1;
    }

    SQInteger _readActionHandle(HSQUIRRELVM vm, SQInteger idx, ActionHandle* outHandle){
        SQUserPointer pointer = 0;
        SQUserPointer typeTag = 0;
        sq_getuserdata(vm, idx, &pointer, &typeTag);
        if(!pointer) return sq_throwerror(vm, "Unable to read data from compiled dialog.");
        if(typeTag != DigitalActionHandleTypeTag) return sq_throwerror(vm, "Incorrect object passed as action handle.");

        ActionHandle* actionHandle = static_cast<ActionHandle*>(pointer);
        *outHandle = *actionHandle;

        return 0;
    }

    SQInteger InputNamespace::getDigitalAction(HSQUIRRELVM vm){
        ActionHandle handle = INVALID_ACTION_HANDLE;
        SQInteger readResult = _readActionHandle(vm, -1, &handle);
        if(readResult != 0) return readResult;

        bool result = BaseSingleton::getInputManager()->getDigitalAction(0, handle);

        sq_pushbool(vm, result);
        return 1;
    }

    void InputNamespace::setupNamespace(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, getKey, "getKey", 2, ".i");
        ScriptUtils::addFunction(vm, getMouseX, "getMouseX");
        ScriptUtils::addFunction(vm, getMouseY, "getMouseY");
        ScriptUtils::addFunction(vm, getMouseButton, "getMouseButton", 2, ".i");

        ScriptUtils::addFunction(vm, getDigitalActionHandle, "getDigitalActionHandle", 2, ".s");
        ScriptUtils::addFunction(vm, getDigitalAction, "getDigitalAction", 2, ".u");
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
