#ifdef DEBUGGING_TOOLS

#pragma once

#include "ScriptUtils.h"

namespace AV{
    class DeveloperNamespace{
    public:
        DeveloperNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger setMeshGroupVisible(HSQUIRRELVM vm);
    };
}

#endif
