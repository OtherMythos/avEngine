#include "TimerNamespace.h"

#include "System/BaseSingleton.h"
#include "System/Timing/TimerManager.h"

namespace AV{

    SQInteger TimerNamespace::startCountdown(HSQUIRRELVM vm){
        SQInteger numMilliseconds;
        sq_getinteger(vm, 2, &numMilliseconds);

        SQObject targetClosure;
        sq_resetobject(&targetClosure);
        sq_getstackobj(vm, 3, &targetClosure);
        //Assuming the param check worked.
        assert(targetClosure._type == OT_CLOSURE);

        //Check the provided closure is of the correct format.
        SQInteger numParams, numFreeVariables;
        sq_getclosureinfo(vm, 3, &numParams, &numFreeVariables);
        if(numParams != 1) return sq_throwerror(vm, "Provided function must take no parameters.");

        SQObject targetContext;
        sq_resetobject(&targetContext);

        SQInteger size = sq_gettop(vm);
        if(size >= 4){
            sq_getstackobj(vm, 4, &targetContext);
        }

        BaseSingleton::getTimerManager()->addCountdownTimer(numMilliseconds, targetClosure, targetContext);

        return 0;
    }

    /**SQNamespace
    @name _mesh
    @desc Functions to create meshes.
    */
    void TimerNamespace::setupNamespace(HSQUIRRELVM vm){

        /**SQFunction
        @name countdown
        @desc Begin a countdown timer. When the countdown time is finished the provided context will be called.
        @param1:time: A number representing the number of milliseconds until the closure should be called.
        @param2:closure: The target closure to call.
        */
        ScriptUtils::addFunction(vm, startCountdown, "countdown", -3, ".nct|x");
    }
}
