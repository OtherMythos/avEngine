#pragma once

#include "World/Physics/PhysicsShapeManager.h"
#include "World/Physics/Worlds/CollisionWorld.h"

namespace AV{
    struct CollisionComponent{
        CollisionComponent(PhysicsTypes::CollisionObjectPtr a, PhysicsTypes::CollisionObjectPtr b) : objA(a), objB(b) { }
        ~CollisionComponent(){
            objA.reset();
            objB.reset();
        }

        PhysicsTypes::CollisionObjectPtr objA;
        PhysicsTypes::CollisionObjectPtr objB;

    };
}
