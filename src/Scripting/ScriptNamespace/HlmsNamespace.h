#pragma once

#include "ScriptNamespace.h"

namespace AV {
    class HlmsNamespace : public ScriptNamespace{
    public:
        HlmsNamespace() {};

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger PBSCreateDatablock(HSQUIRRELVM vm);
    };
}
