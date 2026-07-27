#pragma once

#include "ScriptUtils.h"

namespace AV{
    class EntityNamespace{
    public:
        EntityNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger createEntity(HSQUIRRELVM vm);
        static SQInteger destroyEntity(HSQUIRRELVM vm);

    };
}
