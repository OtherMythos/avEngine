#pragma once

#include "World/Physics/PhysicsShapeManager.h"
#include "World/Physics/Worlds/DynamicsWorld.h"

namespace AV{
    struct RigidBodyComponent{
        RigidBodyComponent(PhysicsBodyConstructor::RigidBodyPtr b) : body(b) { }
        ~RigidBodyComponent(){
            body.reset();
        }

        PhysicsBodyConstructor::RigidBodyPtr body;

    };
}
