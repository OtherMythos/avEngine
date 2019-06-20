#pragma once

#include "ComponentLogic.h"

#include "World/Physics/Worlds/DynamicsWorld.h"

#include "OgreString.h"

namespace AV{
    class SerialiserStringStore;

    class RigidBodyComponentLogic : public ComponentLogic{
    public:
        static bool add(eId id, PhysicsBodyConstructor::RigidBodyPtr body);
        static bool remove(eId id);

        static bool getBody(eId id, PhysicsBodyConstructor::RigidBodyPtr& body);

        static void serialise(std::ofstream& stream, entityx::Entity& e);
        static void deserialise(eId entity, std::ifstream& file, SerialiserStringStore *store);
    };
}
