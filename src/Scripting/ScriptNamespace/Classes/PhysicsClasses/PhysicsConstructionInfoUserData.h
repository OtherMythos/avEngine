#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"

#include "World/Physics/PhysicsTypes.h"

namespace AV{
    /**
    A class to store collision object data.
    Storing it with this method means less parsing is needed later on.
    */
    class PhysicsConstructionInfoUserData{
    public:
        PhysicsConstructionInfoUserData() = delete;
        ~PhysicsConstructionInfoUserData() = delete;

        static void dynamicConstructionInfoFromData(HSQUIRRELVM vm, const btRigidBody::btRigidBodyConstructionInfo& info);
        static UserDataGetResult getDynamicConstructionInfo(HSQUIRRELVM vm, SQInteger index, btRigidBody::btRigidBodyConstructionInfo& outInfo);

    };
}
