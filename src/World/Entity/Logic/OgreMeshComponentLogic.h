#pragma once

#include "ComponentLogic.h"

#include "World/Slot/SlotPosition.h"
#include "World/Support/OgreMeshManager.h"
#include "OgreString.h"

namespace entityx{
    class Entity;
}

namespace AV{
    class SerialiserStringStore;

    class OgreMeshComponentLogic : public ComponentLogic{
    public:
        static void add(eId id, const Ogre::String &mesh);
        static void add(eId id, OgreMeshManager::OgreMeshPtr mesh);
        static bool remove(eId id);

        static void reposition(eId id);
        static void repositionKnown(eId id, const SlotPosition& pos);

        static void orientate(eId id, Ogre::Quaternion orientation);

        static OgreMeshManager::OgreMeshPtr getMesh(eId id);
        static void serialise(std::ofstream& stream, entityx::Entity& e);
        static void deserialise(eId entity, std::ifstream& file, SerialiserStringStore* store);

    private:
        static void _add(entityx::Entity& entity, OgreMeshManager::OgreMeshPtr mesh);
    };
}
