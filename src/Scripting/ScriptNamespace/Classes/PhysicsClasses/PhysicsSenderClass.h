#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "World/Physics/PhysicsTypes.h"

namespace AV{
    /**
    A class to expose collision world sender objects to squirrel.
    */
    class PhysicsSenderClass{
    public:
        PhysicsSenderClass() = delete;

        static void setupClass(HSQUIRRELVM vm);

        static void createInstanceFromPointer(HSQUIRRELVM vm, PhysicsTypes::CollisionObjectPtr shape, bool receiver);
        enum GetCollisionObjectType{
            EITHER,
            RECEIVER,
            SENDER
        };
        static bool getPointerFromInstance(HSQUIRRELVM vm, SQInteger index, PhysicsTypes::CollisionObjectPtr* outPtr, GetCollisionObjectType getType);

        static SQInteger setObjectPosition(HSQUIRRELVM vm);

    private:
        static SQInteger physicsObjectReleaseHook(SQUserPointer p, SQInteger size);

        static SQObject senderDelegateTable;
        static SQObject receiverDelegateTable;
    };
}
