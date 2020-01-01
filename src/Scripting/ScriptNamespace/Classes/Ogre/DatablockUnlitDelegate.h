#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class DatablockUnlitDelegate{
    public:
        DatablockUnlitDelegate() = delete;

        static void setupTable(HSQUIRRELVM vm);

    private:
        static SQInteger setColour(HSQUIRRELVM vm);
        static SQInteger setUseColour(HSQUIRRELVM vm);
        static SQInteger setTexture(HSQUIRRELVM vm);
    };
}
