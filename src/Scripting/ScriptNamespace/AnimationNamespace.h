#pragma once

#include "ScriptUtils.h"

namespace AV{
    class AnimationNamespace{
    public:
        AnimationNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger createAnimation(HSQUIRRELVM vm);
        static SQInteger createAnimationInfo(HSQUIRRELVM vm);
        static SQInteger loadAnimationFile(HSQUIRRELVM vm);
        static SQInteger getNumActiveAnimations(HSQUIRRELVM vm);
    };
}
