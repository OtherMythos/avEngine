#pragma once

#include "ScriptNamespace.h"

namespace AV{
    class ScriptManager;

    class MeshNamespace : public ScriptNamespace{
        friend ScriptManager;
    public:
        MeshNamespace() {};

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger createMesh(HSQUIRRELVM vm);
    };
}
