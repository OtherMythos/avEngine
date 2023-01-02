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
        static SQInteger setRenderQueueForMeshGroup(HSQUIRRELVM vm);

        static SQInteger drawPoint(HSQUIRRELVM vm);
        static SQInteger drawAxis(HSQUIRRELVM vm);
        static SQInteger drawCircle(HSQUIRRELVM vm);
        static SQInteger drawSphere(HSQUIRRELVM vm);
    };
}

#endif
