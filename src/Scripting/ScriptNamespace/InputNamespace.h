#pragma once

#include "ScriptUtils.h"

namespace AV {
    class InputNamespace{
    public:
        InputNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

        static void setupConstants(HSQUIRRELVM vm);

    private:
        static SQInteger getKey(HSQUIRRELVM vm);

        static SQInteger getMouseX(HSQUIRRELVM vm);
        static SQInteger getMouseY(HSQUIRRELVM vm);
        static SQInteger getMouseButton(HSQUIRRELVM vm);

        static SQInteger getDigitalActionHandle(HSQUIRRELVM vm);
        static SQInteger getDigitalAction(HSQUIRRELVM vm);

        static SQInteger setActionSets(HSQUIRRELVM vm);
    };
}
