#pragma once

#include "ScriptUtils.h"

namespace AV{
    class StateNamespace{
    public:
        StateNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

    private:
        static SQInteger setPauseState(HSQUIRRELVM vm);
        static SQInteger getPauseState(HSQUIRRELVM vm);
    };
}
