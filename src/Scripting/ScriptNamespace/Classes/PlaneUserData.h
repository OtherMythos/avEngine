#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class PlaneUserData{
    public:
        PlaneUserData() = delete;
        ~PlaneUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void planeToUserData(HSQUIRRELVM vm, const Ogre::Plane& vec);
        static UserDataGetResult readPlaneFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Plane* outPlane);

    private:
        static SQInteger planeToString(HSQUIRRELVM vm);
        static SQInteger planeCompare(HSQUIRRELVM vm);
        static SQInteger copy(HSQUIRRELVM vm);

        static SQObject planeDelegateTableObject;

        static SQInteger createPlane(HSQUIRRELVM vm);

        static UserDataGetResult _readPlanePtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Plane** outPlane);
    };
}
