#pragma once

#include "PhysicsObjectClass.h"
#include "World/Physics/PhysicsShapeManager.h"
#include "World/Physics/Worlds/DynamicsWorld.h"
#include "Scripting/ScriptDataPacker.h"

class btRigidBody;

namespace AV{
    class PhysicsRigidBodyClass : public PhysicsObjectClass{
    public:
        PhysicsRigidBodyClass();
        ~PhysicsRigidBodyClass();

        static void setupClass(HSQUIRRELVM vm);

        static void _createInstanceFromInfo(HSQUIRRELVM vm, DynamicsWorld::RigidBodyPtr body);
        static DynamicsWorld::RigidBodyPtr getRigidBodyFromInstance(HSQUIRRELVM vm, SQInteger index);

    private:
        static SQObject classObject;

        static SQInteger sqPhysicsRigidBodyReleaseHook(SQUserPointer p, SQInteger size);

        static SQInteger bodyInWorld(HSQUIRRELVM vm);

        static ScriptDataPacker<DynamicsWorld::RigidBodyPtr> mBodyData;
    };
}
