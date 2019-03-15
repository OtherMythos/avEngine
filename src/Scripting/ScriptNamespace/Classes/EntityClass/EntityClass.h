#pragma once

#include <squirrel.h>

namespace AV{
    class EntityClass{
    public:
        EntityClass() { };

        void setupClass(HSQUIRRELVM vm);
    };
}
