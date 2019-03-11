#pragma once

#include "OgreString.h"

namespace Ogre{
    class SceneNode;
    class SceneManager;
}

namespace AV{
    class OgreMeshManager{
    public:
        OgreMeshManager();
        ~OgreMeshManager();

        Ogre::SceneNode* createOgreMesh(const Ogre::String& meshName);

    private:
        Ogre::SceneNode* mParentEntityNode;
        Ogre::SceneManager* mSceneManager;
    };
}
