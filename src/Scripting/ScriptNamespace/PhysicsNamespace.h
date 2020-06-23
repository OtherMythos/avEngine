#pragma once

#include "ScriptUtils.h"

#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "World/Physics/Worlds/CollisionWorldUtils.h"

namespace AV {
    class PhysicsNamespace{
    public:
        PhysicsNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger getCubeShape(HSQUIRRELVM vm);
        static SQInteger getSphereShape(HSQUIRRELVM vm);
        static SQInteger getCapsuleShape(HSQUIRRELVM vm);

        static SQInteger createRigidBody(HSQUIRRELVM vm);
        static SQInteger addRigidBody(HSQUIRRELVM vm);
        static SQInteger removeRigidBody(HSQUIRRELVM vm);

        static SQInteger createCollisionSender(HSQUIRRELVM vm);
        static SQInteger createCollisionReceiver(HSQUIRRELVM vm);
        static SQInteger addCollisionObject(HSQUIRRELVM vm);

        static SQInteger _createCollisionObject(HSQUIRRELVM vm, CollisionObjectType::CollisionObjectType objType);

        static void _iterateConstructionInfoTable(HSQUIRRELVM vm, SQInteger tableIndex, btRigidBody::btRigidBodyConstructionInfo& info);

        struct SenderConstructionInfo{
            const char* filePath;
            const char* funcName;
            int userId;
            CollisionObjectTypeMask::CollisionObjectTypeMask objType;
            CollisionObjectEventMask::CollisionObjectEventMask eventType;
        };
        static void _iterateSenderConstructionTable(HSQUIRRELVM vm, SQInteger idx, SenderConstructionInfo* outInfo);
    };
}
