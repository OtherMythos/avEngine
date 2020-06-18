#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "World/Physics/PhysicsTypes.h"
#include "Scripting/ScriptDataPacker.h"

namespace AV{
    /**
    A class to expose collision world sender objects to squirrel.
    */
    class PhysicsSenderClass{
    public:
        PhysicsSenderClass() = delete;

        static void setupClass(HSQUIRRELVM vm);

        static void createInstanceFromPointer(HSQUIRRELVM vm, PhysicsTypes::CollisionObjectPtr shape, bool receiver);
        static bool getPointerFromInstance(HSQUIRRELVM vm, SQInteger index, PhysicsTypes::CollisionObjectPtr* outPtr, bool receiver);

    private:
        static SQInteger physicsObjectReleaseHook(SQUserPointer p, SQInteger size);

        static ScriptDataPacker<PhysicsTypes::CollisionObjectPtr> mObjectData;

        static SQObject senderClassObject;
        static SQObject receiverClassObject;
    };
}
