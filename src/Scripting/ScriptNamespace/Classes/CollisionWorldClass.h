#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{

    class CollisionWorldObject;

    /**
    Abstracts a simple file interface for squirrel.
    */
    class CollisionWorldClass{
    public:
        CollisionWorldClass() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

    private:
        enum CollisionWorldType{
            WorldBruteForce,
        };

        static SQObject collisionWorldDelegateTableObject;

        static SQInteger createCollisionWorld(HSQUIRRELVM vm);
        static SQInteger processCollision(HSQUIRRELVM vm);
        static SQInteger addCollisionPoint(HSQUIRRELVM vm);
        static SQInteger removeCollisionPoint(HSQUIRRELVM vm);
        static SQInteger getNumCollisions(HSQUIRRELVM vm);
        static SQInteger getCollisionPairForIdx(HSQUIRRELVM vm);
        static SQInteger setPositionForPoint(HSQUIRRELVM vm);

        static SQInteger collisionWorldReleaseHook(SQUserPointer p, SQInteger size);


        static void collisionWorldToUserData(HSQUIRRELVM vm, CollisionWorldObject* world);
        static UserDataGetResult readCollisionWorldFromUserData(HSQUIRRELVM vm, SQInteger stackInx, CollisionWorldObject** outWorld);
    };
}
