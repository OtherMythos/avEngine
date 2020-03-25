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
        static SQInteger getKey(HSQUIRRELVM vm);

        static SQInteger getMouseX(HSQUIRRELVM vm);
        static SQInteger getMouseY(HSQUIRRELVM vm);
        static SQInteger getMouseButton(HSQUIRRELVM vm);

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
    };
}
