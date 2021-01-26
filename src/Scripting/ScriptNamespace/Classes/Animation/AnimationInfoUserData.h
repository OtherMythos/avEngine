#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Animation/AnimationData.h"

namespace AV{
    class AnimationInfoUserData{
    public:
        AnimationInfoUserData() = delete;

        static void blockPtrToUserData(HSQUIRRELVM vm, AnimationInfoBlockPtr ptr);

        static UserDataGetResult readBlockPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, AnimationInfoBlockPtr* outObject);

    private:
        static SQInteger AnimationObjectReleaseHook(SQUserPointer p, SQInteger size);
    };
}
