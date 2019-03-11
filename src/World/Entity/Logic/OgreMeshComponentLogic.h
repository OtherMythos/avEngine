#pragma once

#include "ComponentLogic.h"
#include "World/Entity/eId.h"

#include "OgreString.h"

namespace AV{
    class OgreMeshComponentLogic : public ComponentLogic{
    public:
        static void add(eId id, const Ogre::String &mesh);
    };
}
