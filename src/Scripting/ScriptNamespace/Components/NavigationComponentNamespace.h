#pragma once

#include "squirrel.h"

namespace AV{
    class NavigationComponentNamespace{
    public:
        NavigationComponentNamespace() {}

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger add(HSQUIRRELVM v);
        static SQInteger remove(HSQUIRRELVM v);
    };
}

