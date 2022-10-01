#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class AudioSourceComponentNamespace{
    public:
        AudioSourceComponentNamespace() {}

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger add(HSQUIRRELVM v);
        static SQInteger remove(HSQUIRRELVM v);
        static SQInteger get(HSQUIRRELVM vm);
        static SQInteger set(HSQUIRRELVM vm);
    };
}
