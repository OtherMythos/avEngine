#pragma once

#include "ComponentLogic.h"

#include "OgreString.h"

namespace AV{
    class ScriptComponentLogic : public ComponentLogic{
    public:
        static void add(eId id, const Ogre::String &scriptPath);
        static bool remove(eId id);
    };
}