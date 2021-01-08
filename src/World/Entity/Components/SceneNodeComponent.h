#pragma once

namespace Ogre{
    class SceneNode;
}

namespace AV{
    struct SceneNodeComponent{
        SceneNodeComponent(Ogre::SceneNode* n)
            : node(n) {}

        Ogre::SceneNode* node;
    };
}
