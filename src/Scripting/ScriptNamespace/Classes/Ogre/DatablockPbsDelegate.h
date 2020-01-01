#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class DatablockPbsDelegate{
    public:
        DatablockPbsDelegate() = delete;

        static void setupTable(HSQUIRRELVM vm);

    private:
        static SQInteger setDiffuse(HSQUIRRELVM vm);
        static SQInteger setMetalness(HSQUIRRELVM vm);
    };
}
