#pragma once

#include "PhysicsObjectClass.h"
#include "World/Physics/PhysicsShapeManager.h"
#include "World/Physics/PhysicsBodyConstructor.h"
#include "Scripting/ScriptDataPacker.h"

class btRigidBody;

namespace AV{
    class PhysicsRigidBodyClass : public PhysicsObjectClass{
    public:
        PhysicsRigidBodyClass();
        ~PhysicsRigidBodyClass();

        static void setupClass(HSQUIRRELVM vm);

        static void createInstanceFromPointer(HSQUIRRELVM vm, PhysicsBodyConstructor::RigidBodyPtr body);
        static PhysicsBodyConstructor::RigidBodyPtr getRigidBodyFromInstance(HSQUIRRELVM vm, SQInteger index);

    private:
        static SQObject classObject;

        static SQInteger sqPhysicsRigidBodyReleaseHook(SQUserPointer p, SQInteger size);

        static SQInteger bodyInWorld(HSQUIRRELVM vm);
        static SQInteger bodyBoundType(HSQUIRRELVM vm);
        static SQInteger rigidBodyCompare(HSQUIRRELVM vm);
        static SQInteger getBodyShape(HSQUIRRELVM vm);

        static ScriptDataPacker<PhysicsBodyConstructor::RigidBodyPtr> mBodyData;
    };
}
