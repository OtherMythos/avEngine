#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class AnimationComponentNamespace{
    public:
        AnimationComponentNamespace() {}

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger add(HSQUIRRELVM v);
        static SQInteger remove(HSQUIRRELVM v);
        static SQInteger get(HSQUIRRELVM vm);
    };
}
