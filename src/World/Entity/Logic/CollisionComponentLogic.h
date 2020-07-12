#pragma once

#include "ComponentLogic.h"

#include "World/Physics/Worlds/CollisionWorld.h"

namespace AV{
    class SerialiserStringStore;

    class CollisionComponentLogic : public ComponentLogic{
    public:
        static bool add(eId id, PhysicsTypes::CollisionObjectPtr a, PhysicsTypes::CollisionObjectPtr b, bool aPopulated, bool bPopulated);
        static bool remove(eId id);

        static bool getBody(eId id, bool a, PhysicsTypes::CollisionObjectPtr* outObj);

        /**
        Reposition the collision objects attached to an entity, assuming checks have already been performed as to whether this object has a collision component.
        */
        static void repositionKnown(eId id, const Ogre::Vector3& pos);
    };
}
