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

        static void _createInstanceFromInfo(HSQUIRRELVM vm, DynamicsWorld::RigidBodyPtr body, PhysicsShapeManager::ShapePtr shapePtr);
        static DynamicsWorld::RigidBodyPtr getRigidBodyFromInstance(HSQUIRRELVM vm, SQInteger index);

    private:
        struct RigidBodyInfo{
            DynamicsWorld::RigidBodyPtr body;
            PhysicsShapeManager::ShapePtr shapePtr;
        };

        static SQObject classObject;

        static SQInteger sqPhysicsRigidBodyReleaseHook(SQUserPointer p, SQInteger size);

        static SQInteger bodyValid(HSQUIRRELVM vm);

        static ScriptDataPacker<RigidBodyInfo> mBodyData;
    };
}
