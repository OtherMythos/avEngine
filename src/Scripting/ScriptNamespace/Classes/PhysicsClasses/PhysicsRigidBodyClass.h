#pragma once

#include <squirrel.h>
#include "World/Physics/PhysicsShapeManager.h"
#include "World/Physics/PhysicsBodyConstructor.h"
#include "Scripting/ScriptDataPacker.h"

class btRigidBody;

namespace AV{
    class PhysicsRigidBodyClass{
    public:
        PhysicsRigidBodyClass() = delete;

        static void setupClass(HSQUIRRELVM vm);

        static void createInstanceFromPointer(HSQUIRRELVM vm, PhysicsTypes::RigidBodyPtr body);
        static PhysicsTypes::RigidBodyPtr getRigidBodyFromInstance(HSQUIRRELVM vm, SQInteger index);

    private:
        static SQObject classObject;

        static SQInteger sqPhysicsRigidBodyReleaseHook(SQUserPointer p, SQInteger size);

        static SQInteger bodyInWorld(HSQUIRRELVM vm);
        static SQInteger bodyBoundType(HSQUIRRELVM vm);
        static SQInteger rigidBodyCompare(HSQUIRRELVM vm);
        static SQInteger getBodyShape(HSQUIRRELVM vm);
        static SQInteger setBodyPosition(HSQUIRRELVM vm);
        static SQInteger setLinearFactor(HSQUIRRELVM vm);
        static SQInteger setLinearVelocity(HSQUIRRELVM vm);
        static SQInteger getBodyPosition(HSQUIRRELVM vm);
        static SQInteger getBodyLinearVelocity(HSQUIRRELVM vm);

        static ScriptDataPacker<PhysicsTypes::RigidBodyPtr> mBodyData;
    };
}
