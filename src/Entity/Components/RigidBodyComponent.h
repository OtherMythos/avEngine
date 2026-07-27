#pragma once

#include "Physics/PhysicsShapeManager.h"
#include "Physics/Worlds/DynamicsWorld.h"

namespace AV{
    struct RigidBodyComponent{
        RigidBodyComponent(PhysicsTypes::RigidBodyPtr b) : body(b) { }
        ~RigidBodyComponent(){
            body.reset();
        }

        PhysicsTypes::RigidBodyPtr body;

    };
}
