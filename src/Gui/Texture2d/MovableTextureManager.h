#pragma once

#include "OgreString.h"
#include <memory>

namespace Ogre{
    class SceneManager;
    class SceneNode;
}

namespace AV{
    class MovableTexture;

    typedef std::shared_ptr<MovableTexture> MovableTexturePtr;

    /**
    A class to allow the creation of simple 2d texture renderables.
    This class abstracts parts of the logic found in the Rect2dRenderables and datablock creation and management to provide a simple interface to create textures.
    */
    class MovableTextureManager{
    public:
        MovableTextureManager();
        ~MovableTextureManager();

        //Initialise the texture manager, creating a scene node to hold all the textures later on.
        void initialise(Ogre::SceneManager* sceneManager);

        MovableTexturePtr createTexture(const Ogre::String& resourceName, const Ogre::String& resourceGroup = "General");

        static void _destroyMovableTexture(MovableTexture* body);

    private:
        Ogre::SceneManager* mSceneManager;
        Ogre::SceneNode* mParentNode;
    };
}
