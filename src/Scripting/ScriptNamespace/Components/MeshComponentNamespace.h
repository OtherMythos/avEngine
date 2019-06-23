#pragma once

#include "squirrel.h"

namespace AV{
    class MeshComponentNamespace{
    public:
        MeshComponentNamespace() {}

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger add(HSQUIRRELVM v);
        static SQInteger remove(HSQUIRRELVM v);
        static SQInteger getMesh(HSQUIRRELVM v);
    };
}
