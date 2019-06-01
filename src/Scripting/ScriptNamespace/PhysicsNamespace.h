#pragma once

#include "ScriptNamespace.h"

namespace AV {
    class PhysicsNamespace : public ScriptNamespace{
    public:
        PhysicsNamespace() {};
        
        void setupNamespace(HSQUIRRELVM vm);
        
    private:
        static SQInteger obtainCubeShape(HSQUIRRELVM vm);
    };
}
