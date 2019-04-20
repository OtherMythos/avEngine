#pragma once

#include "ComponentLogic.h"

#include "World/Slot/SlotPosition.h"
#include "OgreString.h"

namespace AV{
    class OgreMeshComponentLogic : public ComponentLogic{
    public:
        static void add(eId id, const Ogre::String &mesh);
        static bool remove(eId id);

        static void reposition(eId id);
        static void repositionKnown(eId id, const SlotPosition& pos);
    };
}
