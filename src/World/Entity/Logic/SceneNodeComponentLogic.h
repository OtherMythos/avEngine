#pragma once

#include "ComponentLogic.h"
#include "World/Slot/SlotPosition.h"

namespace entityx{
    class Entity;
}

namespace Ogre{
    class SceneNode;
}

namespace AV{

    class SceneNodeComponentLogic : public ComponentLogic{
    public:
        static void add(eId id, Ogre::SceneNode* targetNode);
        static bool remove(eId id);

        static Ogre::SceneNode* getSceneNode(eId id);

        static void repositionKnown(eId id, const SlotPosition& pos);
    };
}
