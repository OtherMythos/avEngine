#pragma once

#include "World/Physics/PhysicsShapeManager.h"
#include "World/Physics/Worlds/DynamicsWorld.h"

namespace AV{
    struct RigidBodyComponent{
        RigidBodyComponent(DynamicsWorld::RigidBodyPtr b) : body(b) { }
        ~RigidBodyComponent(){
            body.reset();
        }

        DynamicsWorld::RigidBodyPtr body;

    };
}
