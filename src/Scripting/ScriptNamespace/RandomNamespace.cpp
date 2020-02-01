#include "RandomNamespace.h"

#include <stdlib.h>
#include <time.h>

namespace AV{
    SQInteger RandomNamespace::randomFloat(HSQUIRRELVM vm){
        float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
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

        SQInteger retNum = min + (rand() % static_cast<int>(max - min + 1));
        sq_pushinteger(vm, retNum);

        return 1;
    }

    void RandomNamespace::setupNamespace(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, randomFloat, "rand");
        ScriptUtils::addFunction(vm, randomIntRange, "randInt", -2, ".ii");

        //Here is as good a place as any to initialise this.
        srand(time(NULL));
    }

}
