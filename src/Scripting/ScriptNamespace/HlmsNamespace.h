#pragma once

#include "ScriptUtils.h"

namespace AV {
    class HlmsNamespace{
    public:
        HlmsNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger PBSCreateDatablock(HSQUIRRELVM vm);

        static SQInteger UnlitCreateDatablock(HSQUIRRELVM vm);
    };
}
