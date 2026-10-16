#pragma once
#ifdef DEBUG_SERVER
#include "ScriptUtils.h"

namespace AV{
    class DebugNamespace{
    public:
        static void setupNamespace(HSQUIRRELVM vm);
    private:
        static SQInteger pauseSimulation(HSQUIRRELVM vm);
    };
}
#endif
