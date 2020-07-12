#pragma once

#include "ComponentLogic.h"

#include "World/Physics/Worlds/CollisionWorld.h"

namespace AV{
    class SerialiserStringStore;

    class CollisionComponentLogic : public ComponentLogic{
    public:
        static bool add(eId id, PhysicsTypes::CollisionObjectPtr a, PhysicsTypes::CollisionObjectPtr b);
        static bool remove(eId id);

        static bool getBody(eId id, bool a, PhysicsTypes::CollisionObjectPtr* outObj);
    };
}
