#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace Ogre{
    class SkeletonInstance;
}

namespace AV{
    class SkeletonUserData{
    public:
        SkeletonUserData();
        ~SkeletonUserData();

        static void skeletonToUserData(HSQUIRRELVM vm, Ogre::SkeletonInstance* object);
        static UserDataGetResult readSkeletonFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::SkeletonInstance** outObject);

        static void setupDelegateTable(HSQUIRRELVM vm);

    private:
        static SQObject SkeletonDelegateTableObject;

        static SQInteger getNumAnimations(HSQUIRRELVM vm);
        static SQInteger getAnimation(HSQUIRRELVM vm);
        static SQInteger getNumBones(HSQUIRRELVM vm);

        static SQInteger getBone(HSQUIRRELVM vm);
        static SQInteger resetToPose(HSQUIRRELVM vm);
    };
}
