#pragma once

#include "squirrel.h"

namespace AV{
    class SceneNodeComponentNamespace{
    public:
        SceneNodeComponentNamespace() {}

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger add(HSQUIRRELVM v);
        static SQInteger remove(HSQUIRRELVM v);
        static SQInteger getNode(HSQUIRRELVM vm);
    };
}

