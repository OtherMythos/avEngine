#pragma once

#include "ScriptNamespace.h"

namespace AV{
    class ScriptingStateManager;
    class ScriptManager;

    class ScriptingStateNamespace : public ScriptNamespace{
        friend ScriptManager;
    public:
        ScriptingStateNamespace() {};

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static ScriptingStateManager* stateManager;

        static SQInteger startState(HSQUIRRELVM vm);
        static SQInteger endState(HSQUIRRELVM vm);
    };
}
