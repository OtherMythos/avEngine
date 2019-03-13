#pragma once

#include "ComponentNamespace.h"

namespace AV{
    class MeshComponentNamespace : public ComponentNamespace{
    public:
        MeshComponentNamespace() {}

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger add(HSQUIRRELVM v);
        static SQInteger remove(HSQUIRRELVM v);
    };
}
