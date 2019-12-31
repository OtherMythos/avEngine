#pragma once

#include "ScriptUtils.h"

namespace AV {
    class InputNamespace{
    public:
        InputNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

        static void setupConstants(HSQUIRRELVM vm);

    private:
        static SQInteger getKey(HSQUIRRELVM vm);
    };
}
