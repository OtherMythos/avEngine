#pragma once

#include "ComponentLogic.h"

#include "OgreString.h"

namespace AV{
    class ScriptComponentLogic : public ComponentLogic{
    public:
        static void add(eId id, const Ogre::String &scriptPath);
        static bool remove(eId id);

        static void serialise(std::ofstream& stream, entityx::Entity& e);
        static void deserialise(eId entity, std::ifstream& file);
    };
}
