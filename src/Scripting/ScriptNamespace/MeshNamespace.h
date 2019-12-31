#pragma once

#include "ScriptUtils.h"

namespace AV{
    class MeshNamespace{
    public:
        MeshNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger createMesh(HSQUIRRELVM vm);
    };
}
