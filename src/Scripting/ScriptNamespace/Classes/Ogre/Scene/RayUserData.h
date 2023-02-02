#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace Ogre{
    class Ray;
}

namespace AV{
    class RayUserData{
    public:
        RayUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void RayToUserData(HSQUIRRELVM vm, const Ogre::Ray* object);

        static UserDataGetResult readRayFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Ray* outObject);

    private:

        static SQObject rayDelegateTableObject;

        static SQInteger getDirection(HSQUIRRELVM vm);
        static SQInteger getOrigin(HSQUIRRELVM vm);
        static SQInteger getPoint(HSQUIRRELVM vm);
        static SQInteger intersects(HSQUIRRELVM vm);

        static SQInteger createRay(HSQUIRRELVM vm);

        static UserDataGetResult _readRayPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Ray** outObject);

    };
}
