#pragma once

#include "ScriptNamespace.h"

namespace AV {
    class PhysicsNamespace : public ScriptNamespace{
    public:
        PhysicsNamespace() {};

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger getCubeShape(HSQUIRRELVM vm);
        static SQInteger getSphereShape(HSQUIRRELVM vm);
    };
}
