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

        typedef std::shared_ptr<Ogre::SceneNode> OgreMeshPtr;

        OgreMeshPtr createMesh(const Ogre::String& meshName);

        void setupSceneManager(Ogre::SceneManager* manager);

    private:
        Ogre::SceneNode* mParentEntityNode;
        static Ogre::SceneManager* mSceneManager;

        /**
        Iterate through a scene node and destroy its items.
        This will not destroy the nodes themselves.
        */
        void _iterateAndDestroy(Ogre::SceneNode* node);

        static void _destroyOgreMesh(Ogre::SceneNode* sceneNode);
    };
}
