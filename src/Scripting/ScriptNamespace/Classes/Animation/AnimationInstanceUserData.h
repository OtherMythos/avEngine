#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Animation/AnimationData.h"

namespace AV{
    class AnimationInstanceUserData{
    public:
        AnimationInstanceUserData() = delete;

        static void animationPtrToUserData(HSQUIRRELVM vm, SequenceAnimationPtr ptr);

        static UserDataGetResult readAnimationPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, SequenceAnimationPtr* outObject);

        static void setupDelegateTable(HSQUIRRELVM vm);
    private:
        static SQInteger AnimationObjectReleaseHook(SQUserPointer p, SQInteger size);

        static SQInteger isRunning(HSQUIRRELVM vm);
        static SQInteger getTime(HSQUIRRELVM vm);
        static SQInteger setRunning(HSQUIRRELVM vm);

        static SQObject animDelegateTable;
    };
}
