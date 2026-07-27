#pragma once

#include "ComponentLogic.h"

#include "OgreString.h"

namespace AV{
    class SerialiserStringStore;

    enum class ScriptComponentAddResult{
        SUCCESS,
        ALREADY_HAS_COMPONENT,
        FAILURE
    };

    class ScriptComponentLogic : public ComponentLogic{
    public:
        static ScriptComponentAddResult add(eId id, const Ogre::String &scriptPath);
        static bool remove(eId id);

        static void serialise(std::ofstream& stream, entityx::Entity& e);
        static void deserialise(eId entity, std::ifstream& file, SerialiserStringStore *store);
    };
}
