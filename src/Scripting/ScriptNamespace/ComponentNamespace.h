#pragma once

#include <squirrel.h>

namespace AV{
    class ComponentNamespace{
    public:
        ComponentNamespace() {};

        void setupNamespace(HSQUIRRELVM vm);
    };
}
