#pragma once

#include "ScriptUtils.h"

namespace AV{

    /**
    Miscellaneous functions for squirrel.
    These are not specialised enough that they should be included as part of a namespace.
    */
    class MiscFunctions{
    public:
        MiscFunctions() = delete;

        static void setupFunctions(HSQUIRRELVM vm);

    private:
        static SQInteger doFile(HSQUIRRELVM vm);
        static SQInteger getTime(HSQUIRRELVM vm);
    };
}
