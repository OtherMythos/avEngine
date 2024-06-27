#pragma once

#include "ScriptUtils.h"
#include "Input/InputPrerequisites.h"

namespace AV {
    class InputNamespace{
    public:
        InputNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

        static void setupConstants(HSQUIRRELVM vm);

        static void createActionSetHandleUserData(HSQUIRRELVM vm, ActionSetHandle handle);
        static ActionSetHandle readActionSetHandle(HSQUIRRELVM vm, SQInteger idx);
        static SQInteger readActionHandleUserData(HSQUIRRELVM vm, SQInteger idx, ActionHandle* outHandle);
        static void createActionHandleUserData(HSQUIRRELVM vm, ActionHandle actionHandle);

    private:

        static SQInteger getMouseX(HSQUIRRELVM vm);
        static SQInteger getMouseY(HSQUIRRELVM vm);
        static SQInteger getActualMouseX(HSQUIRRELVM vm);
        static SQInteger getActualMouseY(HSQUIRRELVM vm);
        static SQInteger getMouseButton(HSQUIRRELVM vm);
        static SQInteger getMousePressed(HSQUIRRELVM vm);
        static SQInteger getMouseReleased(HSQUIRRELVM vm);
        static SQInteger getMouseWheelValue(HSQUIRRELVM vm);

        static SQInteger getButtonActionHandle(HSQUIRRELVM vm);
        static SQInteger getAxisActionHandle(HSQUIRRELVM vm);
        static SQInteger getTriggerActionHandle(HSQUIRRELVM vm);

        static SQInteger getButtonAction(HSQUIRRELVM vm);
        static SQInteger getTriggerAction(HSQUIRRELVM vm);
        static SQInteger getAxisActionX(HSQUIRRELVM vm);
        static SQInteger getAxisActionY(HSQUIRRELVM vm);
        static SQInteger _getAxisAction(HSQUIRRELVM vm, bool x);

        static SQInteger setActionSets(HSQUIRRELVM vm);
        static SQInteger getActionSetNames(HSQUIRRELVM vm);
        static SQInteger getActionSetHandle(HSQUIRRELVM vm);
        static SQInteger getActionNamesForSet(HSQUIRRELVM vm);

        static SQInteger getNumControllers(HSQUIRRELVM vm);
        static SQInteger getDeviceName(HSQUIRRELVM vm);
        static SQInteger getMostRecentDevice(HSQUIRRELVM vm);

        static SQInteger setActionSetForDevice(HSQUIRRELVM vm);
        static SQInteger setAxisDeadzone(HSQUIRRELVM vm);
        static SQInteger setDefaultAxisDeadzone(HSQUIRRELVM vm);

        static SQInteger mapControllerInput(HSQUIRRELVM vm);
        static SQInteger mapKeyboardInput(HSQUIRRELVM vm);
        static SQInteger mapKeyboardInputAxis(HSQUIRRELVM vm);
        static SQInteger clearAllMapping(HSQUIRRELVM vm);

        static SQInteger getTouchPosition(HSQUIRRELVM vm);
        static SQInteger rumbleInputDevice(HSQUIRRELVM vm);

        //NOTE taken from the testing namespace originally.
        static SQInteger sendButtonAction(HSQUIRRELVM vm);
        static SQInteger sendTriggerAction(HSQUIRRELVM vm);
        static SQInteger sendAxisAction(HSQUIRRELVM vm);
        static SQInteger sendKeyboardKeyPress(HSQUIRRELVM vm);
    };
}
