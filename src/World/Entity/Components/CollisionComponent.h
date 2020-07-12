#pragma once

#include "World/Physics/PhysicsShapeManager.h"
#include "World/Physics/Worlds/CollisionWorld.h"

namespace AV{
    struct CollisionComponent{
        CollisionComponent(PhysicsTypes::CollisionObjectPtr a, PhysicsTypes::CollisionObjectPtr b, bool aPop, bool bPop) : aPopulated(aPop), bPopulated(bPop), objA(a), objB(b) { }
        ~CollisionComponent(){
            objA.reset();
            objB.reset();
        }

        /**
        Keeping track of whether the object is populated like this is more of a work around than anything.
        I don't have a value which can be used for invalid pointers, and in this case a pointer with 0 is valid.
        OPTIMISATION I can investigate using 0 as the invalid object number and baking it into the script data packer, or just changing the system entirely.
        */
        bool aPopulated;
        bool bPopulated;
        PhysicsTypes::CollisionObjectPtr objA;
        PhysicsTypes::CollisionObjectPtr objB;

    };
}
