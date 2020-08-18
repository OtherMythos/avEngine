#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace Ogre{
    class SkeletonAnimation;
}

namespace AV{
    class SkeletonAnimationUserData{
    public:
        SkeletonAnimationUserData();
        ~SkeletonAnimationUserData();

        static void skeletonAnimationToUserData(HSQUIRRELVM vm, Ogre::SkeletonAnimation* object);
        static UserDataGetResult readSkeletonAnimationFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::SkeletonAnimation** outObject);

        static void setupDelegateTable(HSQUIRRELVM vm);

    private:
        static SQObject SkeletonAnimationDelegateTableObject;

        static SQInteger addTime(HSQUIRRELVM vm);
        static SQInteger setLoop(HSQUIRRELVM vm);
        static SQInteger setEnabled(HSQUIRRELVM vm);

        static SQInteger getName(HSQUIRRELVM vm);
        static SQInteger getBone(HSQUIRRELVM vm);
        static SQInteger getCurrentTime(HSQUIRRELVM vm);
        static SQInteger getCurrentFrame(HSQUIRRELVM vm);
    };
}
