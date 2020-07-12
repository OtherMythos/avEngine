#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class CollisionComponentNamespace{
    public:
        CollisionComponentNamespace() {}

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger add(HSQUIRRELVM v);
        static SQInteger remove(HSQUIRRELVM v);
        static SQInteger getObject(HSQUIRRELVM v);
    };
}
