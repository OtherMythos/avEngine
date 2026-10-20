#pragma once

#include "ComponentLogic.h"

#include "Physics/Worlds/DynamicsWorld.h"

#include "OgreString.h"

namespace AV{
    class RigidBodyComponentLogic : public ComponentLogic{
    public:
        static bool add(eId id, PhysicsTypes::RigidBodyPtr body);
        static bool remove(eId id);

        static bool getBody(eId id, PhysicsTypes::RigidBodyPtr& body);
    };
}
