#pragma once

#include "ScriptUtils.h"

namespace AV{
    class LottieNamespace{
    public:
        LottieNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger createAnimation(HSQUIRRELVM vm);
        static SQInteger createSurface(HSQUIRRELVM vm);
    };
}
