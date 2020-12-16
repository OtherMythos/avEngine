#pragma once

#include "ScriptUtils.h"
#include "System/Timing/TimerManagerData.h"

namespace AV{
    class TimerNamespace{
    public:
        TimerNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger startCountdown(HSQUIRRELVM vm);
        static SQInteger cancelCountdown(HSQUIRRELVM vm);

        static void _createTimerWrapper(HSQUIRRELVM vm, TimerId id);
        static UserDataGetResult _readTimerId(HSQUIRRELVM vm, SQInteger stackInx, TimerId* outObject);
    };
}
