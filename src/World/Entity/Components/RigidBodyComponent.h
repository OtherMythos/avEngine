#pragma once

#include "World/Physics/PhysicsShapeManager.h"
#include "World/Physics/Worlds/DynamicsWorld.h"

#include "Logger/Log.h"

namespace AV{
    struct RigidBodyComponent{
        RigidBodyComponent(DynamicsWorld::RigidBodyPtr b) : body(b) { }
        ~RigidBodyComponent(){
            AV_INFO("Destroy");
            body.reset();
        }

        DynamicsWorld::RigidBodyPtr body;

    };
}
