#pragma once

#include "OgreString.h"
#include "OgreVector3.h"

namespace Ogre{
    class SceneNode;
    class SceneManager;
}

namespace AV{
    class Event;

    class OgreMeshManager{
    public:
        OgreMeshManager();
        ~OgreMeshManager();

        typedef std::shared_ptr<Ogre::SceneNode> OgreMeshPtr;

        OgreMeshPtr createMesh(const Ogre::String& meshName);

        void setupSceneManager(Ogre::SceneManager* manager);

        bool worldEventReceiver(const Event &e);

    private:
        Ogre::SceneNode* mParentEntityNode;
        static Ogre::SceneManager* mSceneManager;

        /**
        Iterate through a scene node and destroy its items.
        This will not destroy the nodes themselves.
        */
        void _iterateAndDestroy(Ogre::SceneNode* node);

        void _repositionMeshesOriginShift(Ogre::Vector3 offset);

        static void _destroyOgreMesh(Ogre::SceneNode* sceneNode);
    };
}
