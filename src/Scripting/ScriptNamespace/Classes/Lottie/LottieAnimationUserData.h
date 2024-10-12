#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace rlottie{
    class Animation;
}

namespace AV{

    typedef std::shared_ptr<rlottie::Animation> LottieAnimationPtr;

    class LottieAnimationUserData{
    public:
        LottieAnimationUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void lottieAnimationToUserData(HSQUIRRELVM vm, LottieAnimationPtr animation);

        static UserDataGetResult readLottieAnimationFromUserData(HSQUIRRELVM vm, SQInteger stackInx, LottieAnimationPtr* outObject);

    private:

        static SQObject lottieAnimationDelegateTableObject;

        static SQInteger totalFrame(HSQUIRRELVM vm);
        static SQInteger renderSync(HSQUIRRELVM vm);
        static SQInteger lottieAnimationToString(HSQUIRRELVM vm);

        static SQInteger sqReleaseHook(SQUserPointer p, SQInteger size);

    };
}
