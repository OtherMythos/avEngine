#pragma once

#include "ScriptUtils.h"

namespace AV{
    class ScriptVM;
    class SystemNamespace{
        friend ScriptVM;
    public:
        SystemNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:

        static SQInteger makeDirectory(HSQUIRRELVM vm);
    };
}
