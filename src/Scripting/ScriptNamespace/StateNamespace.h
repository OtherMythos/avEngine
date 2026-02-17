#pragma once

#include "ScriptUtils.h"

namespace AV{
    class StateNamespace{
    public:
        StateNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

    private:
        static SQInteger setPauseState(HSQUIRRELVM vm);
        static SQInteger getPauseState(HSQUIRRELVM vm);
        static SQInteger setDefaultAnimationPauseMask(HSQUIRRELVM vm);
        static SQInteger getDefaultAnimationPauseMask(HSQUIRRELVM vm);
        static SQInteger setAnimationPauseMask(HSQUIRRELVM vm);
        static SQInteger getAnimationPauseMask(HSQUIRRELVM vm);
    };
}
