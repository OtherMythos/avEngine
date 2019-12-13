#pragma once

#include "OgreString.h"
#include <memory>
#include <set>

#include "OgrePrerequisites.h"

namespace AV{
    class MovableTexture;
    class Event;
    class CompositorPassRect2d;

    typedef std::shared_ptr<MovableTexture> MovableTexturePtr;

    /**
    A class to allow the creation of simple 2d texture renderables.
    This class abstracts parts of the logic found in the Rect2dRenderables and datablock creation and management to provide a simple interface to create textures.
    */
    class MovableTextureManager{
        friend CompositorPassRect2d;
    public:
        MovableTextureManager();
        ~MovableTextureManager();

        typedef std::set<MovableTexture*> TextureSet;
        typedef Ogre::uint8 LayerId;

        //Initialise the texture manager, creating a scene node to hold all the textures later on.
        void initialise(Ogre::SceneManager* sceneManager);

        MovableTexturePtr createTexture(const Ogre::String& resourceName, const Ogre::String& resourceGroup = "General", LayerId layer = 0);

        void setTextureLayer(MovableTexturePtr tex, LayerId layer);

        static void _destroyMovableTexture(MovableTexture* tex);

        bool systemEventReceiver(const Event& e);

        int getNumTextures() const { return mCurrentTextures.size(); }

    private:
        Ogre::SceneManager* mSceneManager;
        Ogre::SceneNode* mParentNode;

        std::map<LayerId, TextureSet> mCurrentTextures;
    };
}
