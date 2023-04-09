#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "System/Util/Timer/Timer.h"

namespace AV{
    class TimerUserData{
    public:
        TimerUserData() = delete;
        ~TimerUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

    private:
        static SQInteger timerToString(HSQUIRRELVM vm);
        static SQInteger timerCompare(HSQUIRRELVM vm);

        static SQInteger timerStart(HSQUIRRELVM vm);
        static SQInteger timerStop(HSQUIRRELVM vm);
        static SQInteger timerGetSeconds(HSQUIRRELVM vm);

        static SQObject timerDelegateTableObject;

        static SQInteger createTimer(HSQUIRRELVM vm);
        static SQInteger TimerObjectReleaseHook(SQUserPointer p, SQInteger size);

        static void timerToUserData(HSQUIRRELVM vm, Timer* timer);
        static UserDataGetResult readTimerFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Timer** outTimer);
    };
}
