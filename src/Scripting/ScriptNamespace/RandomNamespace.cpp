#include "RandomNamespace.h"

#include "System/EnginePrerequisites.h"
#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Scene/AabbUserData.h"
#include "System/Util/Random/PatternHelper.h"
#include "Math/Simple/OgreAabb.h"

#include <stdlib.h>
#include <time.h>
#include <sqstdblob.h>

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

    SQInteger RandomNamespace::randomVec2(HSQUIRRELVM vm){
        const Ogre::Vector2 vec(_genRandFloat(), _genRandFloat());
        Vector2UserData::vector2ToUserData(vm, vec);

        return 1;
    }

    SQInteger RandomNamespace::randomAABB(HSQUIRRELVM vm){
        Ogre::Aabb aabb;
        AabbUserData::readAabbFromUserData(vm, 2, &aabb);

        SQBool globalSpace = false;
        if(sq_gettop(vm) >= 3){
            sq_getbool(vm, 3, &globalSpace);
        }

        const Ogre::Vector3 start = aabb.mCenter - aabb.mHalfSize;
        const Ogre::Vector3 end = aabb.mCenter + aabb.mHalfSize;

        const Ogre::Vector3 size = end - start;
        const Ogre::Vector3 randVec(_genRandFloat(), _genRandFloat(), _genRandFloat());

        Ogre::Vector3 startOffset = -aabb.mHalfSize;
        if(globalSpace){
            startOffset = start;
        }
        Ogre::Vector3 outVec = ((aabb.mHalfSize * 2) * randVec) + startOffset;

        Vector3UserData::vector3ToUserData(vm, outVec);
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

    SQInteger RandomNamespace::genPerlinNoise(HSQUIRRELVM vm){
        SQInteger width, height;
        sq_getinteger(vm, 2, &width);
        sq_getinteger(vm, 3, &height);
        if(width <= 0 || height <= 0){
            return sq_throwerror(vm, "Width and height must be greater than 0");
        }

        SQFloat frequency = 0.1;
        SQInteger depth = 4;
        if(sq_gettop(vm) >= 4){
            sq_getfloat(vm, 4, &frequency);
        }
        if(sq_gettop(vm) >= 5){
            sq_getinteger(vm, 5, &depth);
        }

        size_t blobSize = sizeof(float) * width * height;
        sqstd_createblob(vm, blobSize);
        SQUserPointer blobData;
        sqstd_getblob(vm, -1, &blobData);

        float* out = static_cast<float*>(blobData);
        PatternHelper::GenPerlinNoise((int)width, (int)height, out, (float)frequency, (int)depth);

        return 1;
    }

    SQInteger RandomNamespace::seedPatternGenerator(HSQUIRRELVM vm){
        SQInteger seedVal;
        sq_getinteger(vm, 2, &seedVal);

        PatternHelper::setPatternSeed((int)seedVal);

        return 0;
    }

    SQInteger RandomNamespace::seed(HSQUIRRELVM vm){
        SQInteger seedVal;
        sq_getinteger(vm, 2, &seedVal);

        srand((uint32)seedVal);

        return 0;
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
        @name randVec2
        @desc Generate Vector2 containing random numbers between 0 and 1.
        */
        ScriptUtils::addFunction(vm, randomVec2, "randVec2");

        /**SQFunction
        @name randAABB
        @desc Generate a Vector3 position in an AABB.
        @param1:AABB:aabb The AABB to generate the random point from.
        @param2:Bool:World Whether to apply the aabb in world space or local.
        */
        ScriptUtils::addFunction(vm, randomAABB, "randAABB", -2, ".ub");

        /**SQFunction
        @name randIndex
        @desc Generate a random index for for a provided array.
        @param1:array:An array to generate an index for. Throws an error if the array has length of 0.
        */
        ScriptUtils::addFunction(vm, randIndex, "randIndex", 2, ".a");

        /**SQFunction
        @name genPerlinNoise
        @desc Generate a set of perlin noise data of specified size.
        @param1:Integer:Width of noise to generate.
        @param2:Integer:Height of noise to generate.
        @param3:float:Frequency of noise.
        @param4:integer:Depth of noise.
        */
        ScriptUtils::addFunction(vm, genPerlinNoise, "genPerlinNoise", -3, ".iini");

        /**SQFunction
        @name seedPatternGenerator
        @desc Seed the pattern generator with a value.
        @param1:integer:seed value to use.
        */
        ScriptUtils::addFunction(vm, seedPatternGenerator, "seedPatternGenerator", 2, ".i");
        /**SQFunction
        @name seed
        @desc Seed the random number generator.
        @param1:integer:seed value to use.
        */
        ScriptUtils::addFunction(vm, seed, "seed", 2, ".i");

        //Here is as good a place as any to initialise this.
        srand((uint32)time(NULL));
        PatternHelper::setPatternSeed((uint32)time(NULL));
    }

}
