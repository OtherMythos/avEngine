#pragma once

#include "ScriptNamespace.h"

namespace AV{
    class EntityNamespace : public ScriptNamespace{
    public:
        EntityNamespace() {};

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger createEntity(HSQUIRRELVM vm);
        static SQInteger createEntityTracked(HSQUIRRELVM vm);
    };
}
