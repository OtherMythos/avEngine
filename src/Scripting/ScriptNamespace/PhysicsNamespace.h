#pragma once

#include "ScriptNamespace.h"

#include "BulletDynamics/Dynamics/btRigidBody.h"

namespace AV {
    class PhysicsNamespace : public ScriptNamespace{
    public:
        PhysicsNamespace() {};

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger getCubeShape(HSQUIRRELVM vm);
        static SQInteger getSphereShape(HSQUIRRELVM vm);

        static SQInteger createRigidBody(HSQUIRRELVM vm);
        static SQInteger addRigidBody(HSQUIRRELVM vm);

        static void _iterateConstructionInfoTable(HSQUIRRELVM vm, SQInteger tableIndex, btRigidBody::btRigidBodyConstructionInfo& info);
    };
}
