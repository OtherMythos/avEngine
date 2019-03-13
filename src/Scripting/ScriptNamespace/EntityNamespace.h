#pragma once

#include "ScriptNamespace.h"

namespace AV{
    class EntityNamespace : public ScriptNamespace{
    public:
        EntityNamespace() {};

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger setEntityPosition(HSQUIRRELVM vm);
        static SQInteger createEntity(HSQUIRRELVM vm);
    };
}
