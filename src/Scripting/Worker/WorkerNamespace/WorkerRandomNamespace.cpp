#include "WorkerRandomNamespace.h"

#include "../ScriptWorker.h"
#include "../ScriptWorkerVM.h"

namespace AV{

    namespace{
        //Every function here needs the owning worker's generator. A worker vm always has an
        //owner in practice; the null check only covers a vm created bare by a test.
        std::mt19937* rngFor(HSQUIRRELVM vm){
            ScriptWorker* worker = ScriptWorkerVM::getOwner(vm);
            return worker ? &worker->rng() : 0;
        }

        SQInteger intInRange(std::mt19937& rng, SQInteger min, SQInteger max){
            if(max < min) std::swap(min, max);
            std::uniform_int_distribution<long long> dist(static_cast<long long>(min), static_cast<long long>(max));
            return static_cast<SQInteger>(dist(rng));
        }
    }

    SQInteger WorkerRandomNamespace::randomFloat(HSQUIRRELVM vm){
        std::mt19937* rng = rngFor(vm);
        if(!rng) return sq_throwerror(vm, "_random is not available outside a script worker run.");

        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        sq_pushfloat(vm, dist(*rng));

        return 1;
    }

    SQInteger WorkerRandomNamespace::randomIntRange(HSQUIRRELVM vm){
        std::mt19937* rng = rngFor(vm);
        if(!rng) return sq_throwerror(vm, "_random is not available outside a script worker run.");

        SQInteger min = 0;
        SQInteger max = 0;
        const SQInteger nargs = sq_gettop(vm);
        if(nargs == 2){
            sq_getinteger(vm, 2, &max);
        }else if(nargs >= 3){
            sq_getinteger(vm, 2, &min);
            sq_getinteger(vm, 3, &max);
        }

        //Matches RandomNamespace, which returns 0 rather than erroring on an empty range.
        if(min == 0 && max == 0){
            sq_pushinteger(vm, 0);
            return 1;
        }

        sq_pushinteger(vm, intInRange(*rng, min, max));

        return 1;
    }

    SQInteger WorkerRandomNamespace::randIndex(HSQUIRRELVM vm){
        std::mt19937* rng = rngFor(vm);
        if(!rng) return sq_throwerror(vm, "_random is not available outside a script worker run.");

        const SQInteger arraySize = sq_getsize(vm, 2);
        if(arraySize <= 0){
            return sq_throwerror(vm, "Array length is 0");
        }

        sq_pushinteger(vm, intInRange(*rng, 0, arraySize - 1));

        return 1;
    }

    SQInteger WorkerRandomNamespace::seed(HSQUIRRELVM vm){
        std::mt19937* rng = rngFor(vm);
        if(!rng) return sq_throwerror(vm, "_random is not available outside a script worker run.");

        SQInteger seedVal = 0;
        sq_getinteger(vm, 2, &seedVal);

        rng->seed(static_cast<std::mt19937::result_type>(seedVal));

        return 0;
    }

    /**SQNamespace
    @name _random
    @desc Random number generation inside a script worker vm. Unlike the main vm's _random this is
    per worker rather than process wide, so seeding one worker does not disturb another and a
    seeded run is reproducible.
    */
    void WorkerRandomNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name rand
        @desc Generate a random number between 0 and 1.
        @returns A float between 0 and 1.
        */
        ScriptUtils::addFunction(vm, randomFloat, "rand");

        /**SQFunction
        @name randInt
        @desc Generate a random integer. With one argument the range is 0 to that value.
        @param1:lowerBound
        @param2:upperBound
        */
        ScriptUtils::addFunction(vm, randomIntRange, "randInt", -2, ".ii");

        /**SQFunction
        @name randIndex
        @desc Generate a random index for a provided array.
        @param1:array:An array to generate an index for. Throws an error if the array has length of 0.
        */
        ScriptUtils::addFunction(vm, randIndex, "randIndex", 2, ".a");

        /**SQFunction
        @name seed
        @desc Seed this worker's random number generator.
        @param1:integer:seed value to use.
        */
        ScriptUtils::addFunction(vm, seed, "seed", 2, ".i");
    }
}
