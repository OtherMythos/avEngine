#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace Ogre{
    class Aabb;
}

namespace AV{
    class AabbUserData{
    public:
        AabbUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void AabbToUserData(HSQUIRRELVM vm, const Ogre::Aabb* object);

        static UserDataGetResult readAabbFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Aabb* outObject);

    private:

        static SQObject aabbDelegateTableObject;

        static SQInteger getCentre(HSQUIRRELVM vm);
        static SQInteger getHalfSize(HSQUIRRELVM vm);
        static SQInteger getSize(HSQUIRRELVM vm);
        static SQInteger getMaximum(HSQUIRRELVM vm);
        static SQInteger getMinimum(HSQUIRRELVM vm);
        static SQInteger getRadius(HSQUIRRELVM vm);
        static SQInteger merge(HSQUIRRELVM vm);
        static SQInteger createAABB(HSQUIRRELVM vm);
        static SQInteger aabbToString(HSQUIRRELVM vm);

        static UserDataGetResult _readAabbPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Aabb** outObject);

    };
}
