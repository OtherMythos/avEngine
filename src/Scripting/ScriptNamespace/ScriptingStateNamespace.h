#pragma once

#include "ScriptUtils.h"

namespace AV{
    class ScriptingStateManager;
    class ScriptManager;

    class ScriptingStateNamespace{
        friend ScriptManager;
    public:
        ScriptingStateNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static ScriptingStateManager* stateManager;

        static SQInteger startState(HSQUIRRELVM vm);
        static SQInteger endState(HSQUIRRELVM vm);
    };
}
