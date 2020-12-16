#include "TimerNamespace.h"

#include "System/BaseSingleton.h"
#include "System/Timing/TimerManager.h"
#include "Scripting/ScriptObjectTypeTags.h"

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

        TimerId id = BaseSingleton::getTimerManager()->addCountdownTimer(numMilliseconds, targetClosure, targetContext);
        _createTimerWrapper(vm, id);

        return 1;
    }

    SQInteger TimerNamespace::cancelCountdown(HSQUIRRELVM vm){
        TimerId outIdx = 0;
        SCRIPT_CHECK_RESULT(_readTimerId(vm, 2, &outIdx));

        BaseSingleton::getTimerManager()->removeCountdownTimer(outIdx);

        return 0;
    }

    void TimerNamespace::_createTimerWrapper(HSQUIRRELVM vm, TimerId id){
        TimerId* pointer = (TimerId*)sq_newuserdata(vm, sizeof(TimerId));
        *pointer = id;

        sq_settypetag(vm, -1, TimerObjectTypeTag);
    }

    UserDataGetResult TimerNamespace::_readTimerId(HSQUIRRELVM vm, SQInteger stackInx, TimerId* outObject){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != TimerObjectTypeTag){
            *outObject = INVALID_TIMER;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        TimerId* p = static_cast<TimerId*>(pointer);
        *outObject = *p;

        return USER_DATA_GET_SUCCESS;
    }

    /**SQNamespace
    @name _timer
    @desc A namespace to trigger time based events.
    */
    void TimerNamespace::setupNamespace(HSQUIRRELVM vm){

        /**SQFunction
        @name countdown
        @desc Begin a countdown timer. When the countdown time is finished the provided context will be called.
        @param1:time: A number representing the number of milliseconds until the closure should be called.
        @param2:closure: The target closure to call.
        */
        ScriptUtils::addFunction(vm, startCountdown, "countdown", -3, ".nct|x");

        ScriptUtils::addFunction(vm, cancelCountdown, "cancelCountdown", 2, ".u");
    }
}
