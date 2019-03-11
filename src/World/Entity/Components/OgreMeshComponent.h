#pragma once

namespace Ogre{
    class SceneNode;
}

namespace AV{
    struct OgreMeshComponent {
        OgreMeshComponent(Ogre::SceneNode *parentNode)
            : parentNode(parentNode) {}

        Ogre::SceneNode *parentNode;
    };
}
