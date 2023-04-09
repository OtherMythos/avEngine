#include "TimerUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include <sstream>

namespace AV{

    SQObject TimerUserData::timerDelegateTableObject;

    void TimerUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 2);

        ScriptUtils::addFunction(vm, timerToString, "_tostring");
        ScriptUtils::addFunction(vm, timerCompare, "_cmp");
        ScriptUtils::addFunction(vm, timerStart, "start");
        ScriptUtils::addFunction(vm, timerStop, "stop");
        ScriptUtils::addFunction(vm, timerGetSeconds, "getSeconds");

        sq_resetobject(&timerDelegateTableObject);
        sq_getstackobj(vm, -1, &timerDelegateTableObject);
        sq_addref(vm, &timerDelegateTableObject);
        sq_pop(vm, 1);

        //Create the creation functions.
        sq_pushroottable(vm);

        {
            ScriptUtils::addFunction(vm, createTimer, "Timer");
        }

        sq_pop(vm, 1);
    }

    SQInteger TimerUserData::createTimer(HSQUIRRELVM vm){
        Timer* t = new Timer();
        timerToUserData(vm, t);

        return 1;
    }

    SQInteger TimerUserData::timerStart(HSQUIRRELVM vm){
        Timer* t = 0;
        readTimerFromUserData(vm, 1, &t);
        assert(t);

        t->start();
        return 0;
    }

    SQInteger TimerUserData::timerStop(HSQUIRRELVM vm){
        Timer* t = 0;
        readTimerFromUserData(vm, 1, &t);
        assert(t);

        t->stop();
        return 0;
    }

    SQInteger TimerUserData::timerGetSeconds(HSQUIRRELVM vm){
        Timer* t = 0;
        readTimerFromUserData(vm, 1, &t);
        assert(t);

        float f = t->getTimeTotal();
        sq_pushfloat(vm, f);
        return 1;
    }

    SQInteger TimerUserData::timerToString(HSQUIRRELVM vm){
        Timer* outTimer;
        SCRIPT_ASSERT_RESULT(readTimerFromUserData(vm, -1, &outTimer));

        std::ostringstream stream;
        stream << *outTimer;
        sq_pushstring(vm, _SC(stream.str().c_str()), -1);

        return 1;
    }

    SQInteger TimerUserData::timerCompare(HSQUIRRELVM vm){
        Timer* first;
        Timer* second;

        SCRIPT_ASSERT_RESULT(readTimerFromUserData(vm, -2, &first));
        SCRIPT_CHECK_RESULT(readTimerFromUserData(vm, -1, &second));

        if(first == second){
            sq_pushinteger(vm, 0);
        }else{
            sq_pushinteger(vm, 2);
        }
        return 1;
    }

    void TimerUserData::timerToUserData(HSQUIRRELVM vm, Timer* timer){
        Timer** pointer = (Timer**)sq_newuserdata(vm, sizeof(Timer*));
        *pointer = timer;

        sq_setreleasehook(vm, -1, TimerObjectReleaseHook);
        sq_pushobject(vm, timerDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, TimerObjectTypeTag);
    }

    SQInteger TimerUserData::TimerObjectReleaseHook(SQUserPointer p, SQInteger size){
        Timer** ptr = static_cast<Timer**>(p);
        delete *ptr;

        return 0;
    }

    UserDataGetResult TimerUserData::readTimerFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Timer** outTimer){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != TimerObjectTypeTag){
            *outTimer = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outTimer = *((Timer**)pointer);

        return USER_DATA_GET_SUCCESS;
    }
}
