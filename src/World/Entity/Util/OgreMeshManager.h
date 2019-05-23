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
        /**
        Destroy an item attached to a scene node.
        This will not destroy the scene node.
        */
        void destroyOgreMesh(Ogre::SceneNode* sceneNode);

    private:
        Ogre::SceneNode* mParentEntityNode;
        Ogre::SceneManager* mSceneManager;
        
        /**
        Iterate through a scene node and destroy its items.
        This will not destroy the nodes themselves.
        */
        void _iterateAndDestroy(Ogre::SceneNode* node);

        void _destroyOgreMesh(Ogre::SceneNode* sceneNode);
    };
}
