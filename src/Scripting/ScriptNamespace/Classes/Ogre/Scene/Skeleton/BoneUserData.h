#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace Ogre{
    class Bone;
}

namespace AV{
    class BoneUserData{
    public:
        BoneUserData();
        ~BoneUserData();

        static void boneToUserData(HSQUIRRELVM vm, Ogre::Bone* object);
        static UserDataGetResult readBoneFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Bone** outObject);

        static void setupDelegateTable(HSQUIRRELVM vm);

    private:
        static SQObject BoneDelegateTableObject;

        static SQInteger setPosition(HSQUIRRELVM vm);
        static SQInteger setScale(HSQUIRRELVM vm);
        static SQInteger setOrientation(HSQUIRRELVM vm);

        static SQInteger getPosition(HSQUIRRELVM vm);
        static SQInteger getScale(HSQUIRRELVM vm);
        static SQInteger getOrientation(HSQUIRRELVM vm);

        static SQInteger getName(HSQUIRRELVM vm);
        static SQInteger getNumChildrenBones(HSQUIRRELVM vm);
        static SQInteger getChildBone(HSQUIRRELVM vm);

        static SQInteger addTagPoint(HSQUIRRELVM vm);
    };
}
