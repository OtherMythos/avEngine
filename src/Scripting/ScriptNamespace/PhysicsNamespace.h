#pragma once

#include "ScriptUtils.h"

#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "World/Physics/Worlds/CollisionWorldUtils.h"
#include "System/EnginePrerequisites.h"

namespace AV {
    class PhysicsNamespace{
    public:
        PhysicsNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

    private:
        static SQInteger getCubeShape(HSQUIRRELVM vm);
        static SQInteger getSphereShape(HSQUIRRELVM vm);
        static SQInteger getCapsuleShape(HSQUIRRELVM vm);

        static SQInteger createRigidBody(HSQUIRRELVM vm);
        static SQInteger addRigidBody(HSQUIRRELVM vm);
        static SQInteger removeRigidBody(HSQUIRRELVM vm);

        template <uint8 A>
        static SQInteger createCollisionSender(HSQUIRRELVM vm);
        template <uint8 A>
        static SQInteger createCollisionReceiver(HSQUIRRELVM vm);
        template <uint8 A>
        static SQInteger addCollisionObject(HSQUIRRELVM vm);
        template <uint8 A>
        static SQInteger removeCollisionObject(HSQUIRRELVM vm);

        static SQInteger _createCollisionObject(HSQUIRRELVM vm, bool isSender, uint8 collisionWorldId);

        static SQInteger collisionWordGetMetamethod(HSQUIRRELVM vm);

        static void _iterateConstructionInfoTable(HSQUIRRELVM vm, SQInteger tableIndex, btRigidBody::btRigidBodyConstructionInfo& info);

        struct SenderConstructionInfo{
            const char* filePath;
            const char* funcName;
            int userId;
            CollisionObjectTypeMask::CollisionObjectTypeMask objType;
            CollisionObjectEventMask::CollisionObjectEventMask eventType;
            SQObject closure;
            uint8 closureParams;
        };
        static void _iterateSenderConstructionTable(HSQUIRRELVM vm, SQInteger idx, SenderConstructionInfo* outInfo);

        static SQInteger setCollisionCallbackOverride(HSQUIRRELVM vm);
    };
}
