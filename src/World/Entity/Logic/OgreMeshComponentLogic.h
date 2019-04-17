#pragma once

#include "ComponentLogic.h"

#include "OgreString.h"

namespace AV{
    class OgreMeshComponentLogic : public ComponentLogic{
    public:
        static void add(eId id, const Ogre::String &mesh);
        static bool remove(eId id);
    };
}
