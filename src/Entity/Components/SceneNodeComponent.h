#pragma once

namespace Ogre{
    class SceneNode;
}

namespace AV{
    struct SceneNodeComponent{
        SceneNodeComponent(Ogre::SceneNode* n, bool destroy = false)
            : node(n), destroyNodeOnDestruction(destroy) {}

        Ogre::SceneNode* node;
        bool destroyNodeOnDestruction;
    };
}
