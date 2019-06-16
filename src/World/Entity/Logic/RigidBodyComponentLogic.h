#pragma once

#include "ComponentLogic.h"

#include "World/Physics/Worlds/DynamicsWorld.h"

#include "OgreString.h"

namespace AV{
    class SerialiserStringStore;

    class RigidBodyComponentLogic : public ComponentLogic{
    public:
        static void add(eId id, DynamicsWorld::RigidBodyPtr body);
        static bool remove(eId id);

        static void serialise(std::ofstream& stream, entityx::Entity& e);
        static void deserialise(eId entity, std::ifstream& file, SerialiserStringStore *store);
    };
}
