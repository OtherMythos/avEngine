#include "RandomNamespace.h"

#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"

#include <stdlib.h>
#include <time.h>

namespace AV{

    #define RAND_INT_MIN_MAX(MIN, MAX) \
        MIN + (rand() % static_cast<int>(MAX - MIN + 1));

    float RandomNamespace::_genRandFloat(){
        return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    }

    SQInteger RandomNamespace::randomFloat(HSQUIRRELVM vm){
        float r = _genRandFloat();
        sq_pushfloat(vm, r);

        return 1;
    }

    SQInteger RandomNamespace::randomIntRange(HSQUIRRELVM vm){
        SQInteger nargs = sq_gettop(vm);
        // float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        // sq_pushfloat(vm, r);
        SQInteger max, min;
        max = min = 0;
        if(nargs == 2){
            sq_getinteger(vm, -1, &max);
        }else if(nargs == 3){
            sq_getinteger(vm, -2, &min);
            sq_getinteger(vm, -1, &max);
        }else assert(false);

        if(max == 0 && min == 0){
            sq_pushinteger(vm, 0);
            return 1;
        }

        SQInteger retNum = RAND_INT_MIN_MAX(min, max);
        sq_pushinteger(vm, retNum);

        return 1;
    }

    SQInteger RandomNamespace::randomVec3(HSQUIRRELVM vm){
        const Ogre::Vector3 vec(_genRandFloat(), _genRandFloat(), _genRandFloat());
        Vector3UserData::vector3ToUserData(vm, vec);

        return 1;
    }

    SQInteger RandomNamespace::randIndex(HSQUIRRELVM vm){
        assert(sq_gettype(vm, 2) == OT_ARRAY);
        SQInteger arraySize = sq_getsize(vm, 2);
        if(arraySize <= 0){
            return sq_throwerror(vm, "Array length is 0");
        }

        SQInteger retNum = RAND_INT_MIN_MAX(0, arraySize-1);
        sq_pushinteger(vm, retNum);

        return 1;
    }

    /**SQNamespace
    @name _random
    @desc A utility namespace to generate random numbers.
    */
    void RandomNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name rand
        @desc Generate a random number between 0 and 1.
        @returns A float between 0 and 1.
        */
        ScriptUtils::addFunction(vm, randomFloat, "rand");
        /**SQFunction
        @name randInt
        @desc Generate a random integer between two bounds. A single input can be provided, which is used as the upper bound, while the lower bound is assumed to be 0.
        @param1:lowerBound
        @param2:upperBound
        */
        ScriptUtils::addFunction(vm, randomIntRange, "randInt", -2, ".ii");

        /**SQFunction
        @name randVec3
        @desc Generate Vector3 containing random numbers between 0 and 1.
        */
        ScriptUtils::addFunction(vm, randomVec3, "randVec3");

        /**SQFunction
        @name randIndex
        @desc Generate a random index for for a provided array.
        @param1:array:An array to generate an index for. Throws an error if the array has length of 0.
        */
        ScriptUtils::addFunction(vm, randIndex, "randIndex", 2, ".a");

        //Here is as good a place as any to initialise this.
        srand(time(NULL));
    }

}
