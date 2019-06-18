#pragma once

#include "squirrel.h"

namespace AV{
    class RigidBodyComponentNamespace{
    public:
        RigidBodyComponentNamespace() {}

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger add(HSQUIRRELVM v);
        static SQInteger remove(HSQUIRRELVM v);
        static SQInteger getRigidBody(HSQUIRRELVM vm);
    };
}
