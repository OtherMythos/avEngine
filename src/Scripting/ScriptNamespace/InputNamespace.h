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
        static SQInteger _readActionHandle(HSQUIRRELVM vm, SQInteger idx, ActionHandle* outHandle);

    private:
        static SQInteger getKey(HSQUIRRELVM vm);

        static SQInteger getMouseX(HSQUIRRELVM vm);
        static SQInteger getMouseY(HSQUIRRELVM vm);
        static SQInteger getMouseButton(HSQUIRRELVM vm);

        static SQInteger getButtonActionHandle(HSQUIRRELVM vm);
        static SQInteger getButtonAction(HSQUIRRELVM vm);

        static SQInteger setActionSets(HSQUIRRELVM vm);
        static SQInteger getActionSetNames(HSQUIRRELVM vm);
        static SQInteger getActionSetHandle(HSQUIRRELVM vm);
        static SQInteger getActionNamesForSet(HSQUIRRELVM vm);
    };
}
