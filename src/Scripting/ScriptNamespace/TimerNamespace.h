#pragma once

#include "ScriptUtils.h"

namespace AV{
    class TimerNamespace{
    public:
        TimerNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger startCountdown(HSQUIRRELVM vm);
    };
}
