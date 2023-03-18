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
        static SQInteger randomVec3(HSQUIRRELVM vm);
        static SQInteger randomVec2(HSQUIRRELVM vm);
        static SQInteger randIndex(HSQUIRRELVM vm);
        static SQInteger genPerlinNoise(HSQUIRRELVM vm);
        static SQInteger seedPatternGenerator(HSQUIRRELVM vm);
        static SQInteger seed(HSQUIRRELVM vm);

        inline static float _genRandFloat();
    };
}
