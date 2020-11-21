#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "World/Physics/PhysicsTypes.h"

namespace AV{
    /**
    A class to expose collision object functionality, using userData.
    */
    class PhysicsObjectUserData{
    public:
        PhysicsObjectUserData() = delete;
        ~PhysicsObjectUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void collisionObjectFromPointer(HSQUIRRELVM vm, PhysicsTypes::CollisionObjectPtr shape, bool receiver);
        enum GetCollisionObjectType{
            EITHER,
            RECEIVER,
            SENDER
        };
        static UserDataGetResult getPointerFromUserData(HSQUIRRELVM vm, SQInteger index, PhysicsTypes::CollisionObjectPtr* outPtr, GetCollisionObjectType getType = EITHER);

        static SQInteger setObjectPosition(HSQUIRRELVM vm);
        static SQInteger getUserIndex(HSQUIRRELVM vm);
        static SQInteger getInternalId(HSQUIRRELVM vm);

    private:
        static SQInteger physicsObjectReleaseHook(SQUserPointer p, SQInteger size);

        static SQObject senderDelegateTable;
        static SQObject receiverDelegateTable;
    };
}
