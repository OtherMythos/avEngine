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
        /**
        @param destroy Whether the stored scene node should be destroyed when the entity is destroyed.
        */
        static void add(eId id, Ogre::SceneNode* targetNode, const Ogre::Vector3& targetPos = Ogre::Vector3::ZERO, bool destroy = false);
        static bool remove(eId id);

        static Ogre::SceneNode* getSceneNode(eId id);

        static void repositionKnown(eId id, const Ogre::Vector3& pos);
    };
}
