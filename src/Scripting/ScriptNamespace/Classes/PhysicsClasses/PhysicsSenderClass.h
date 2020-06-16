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

        static void createInstanceFromPointer(HSQUIRRELVM vm, PhysicsTypes::CollisionSenderPtr shape);
        static bool getPointerFromInstance(HSQUIRRELVM vm, SQInteger index, PhysicsTypes::CollisionSenderPtr* outPtr);

    private:
        static SQInteger physicsSenderReleaseHook(SQUserPointer p, SQInteger size);

        static ScriptDataPacker<PhysicsTypes::CollisionSenderPtr> mObjectData;

        static SQObject classObject;
    };
}
