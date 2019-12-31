#pragma once

#include "ScriptUtils.h"

namespace AV{
    class EntityNamespace{
    public:
        EntityNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger createEntity(HSQUIRRELVM vm);
        static SQInteger createEntityTracked(HSQUIRRELVM vm);
        static SQInteger destroyEntity(HSQUIRRELVM vm);

        static SQInteger trackEntity(HSQUIRRELVM vm);
        static SQInteger untrackEntity(HSQUIRRELVM vm);
    };
}
