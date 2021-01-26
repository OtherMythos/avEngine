#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Animation/AnimationData.h"

namespace AV{
    class AnimationInstanceUserData{
    public:
        AnimationInstanceUserData() = delete;

        static void animationPtrToUserData(HSQUIRRELVM vm, SequenceAnimationPtr ptr);

        static UserDataGetResult readAnimationPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, SequenceAnimationPtr* outObject);

    private:
        static SQInteger AnimationObjectReleaseHook(SQUserPointer p, SQInteger size);
    };
}
