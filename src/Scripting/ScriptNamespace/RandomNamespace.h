#pragma once

#include "ScriptUtils.h"

namespace AV{

    /**
    A namespace providing random number generation functionality.
    I know squirrle comes with one, but you had to do the / maxRand each time you wanted to use it so this is just an easier way to do all that stuff.
    */
    class RandomNamespace{
    public:
        RandomNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger randomFloat(HSQUIRRELVM vm);
        static SQInteger randomIntRange(HSQUIRRELVM vm);
    };
}
