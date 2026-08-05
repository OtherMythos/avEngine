#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{

    /**
    _random inside a script worker vm.

    The engine's RandomNamespace is backed by rand()/srand(), which is process wide state - two
    worker threads drawing from it would interleave, and seeding one would reseed the other. This
    version draws from the owning worker's own std::mt19937, so each worker is independent and a
    seeded run reproduces exactly.

    The function names and typemasks match RandomNamespace so a script moves between the two vms
    unchanged. What is missing is what could not be made to work here: randVec3/randVec2/randAABB
    return userdata, which a worker vm has none of, and genPerlinNoise/seedPatternGenerator go
    through PatternHelper's process wide seed.
    */
    class WorkerRandomNamespace{
    public:
        WorkerRandomNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger randomFloat(HSQUIRRELVM vm);
        static SQInteger randomIntRange(HSQUIRRELVM vm);
        static SQInteger randIndex(HSQUIRRELVM vm);
        static SQInteger seed(HSQUIRRELVM vm);
    };
}
