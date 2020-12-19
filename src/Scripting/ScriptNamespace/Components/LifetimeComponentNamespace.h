#pragma once

#include "squirrel.h"

namespace AV{
    class LifetimeComponentNamespace{
    public:
        LifetimeComponentNamespace() {}

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger add(HSQUIRRELVM v);
        static SQInteger remove(HSQUIRRELVM v);
    };
}

