#pragma once

#include "OgreString.h"
#include <memory>
#include <set>

#include "OgrePrerequisites.h"

namespace AV{
    class Rect2d;
    class Event;
    class CompositorPassRect2d;
    class MovableTexture;

    typedef std::shared_ptr<Rect2d> Rect2dPtr;
    typedef std::shared_ptr<MovableTexture> MovableTexturePtr;

    /**
    A class to allow the creation of simple rect2d renderables.
    This class abstracts parts of the logic found in the Rect2dRenderables to allow easy creation.
    */
    class Rect2dManager{
        friend CompositorPassRect2d;
    public:
        Rect2dManager();
        ~Rect2dManager();

        typedef std::set<Rect2d*> TextureSet;
        typedef Ogre::uint8 LayerId;

        //Initialise the texture manager, creating a scene node to hold all the textures later on.
        void initialise(Ogre::SceneManager* sceneManager);

        Rect2dPtr createRect2d(LayerId layer = 0);
        MovableTexturePtr createTexture(const Ogre::String& resourceName, const Ogre::String& resourceGroup = "General", LayerId layer = 0);

        virtual void setRectLayer(Rect2dPtr rect, LayerId layer);

        static void _destroyRect2d(Rect2d* rect);
        static void _destroyMovableTexture(MovableTexture* tex);

        bool systemEventReceiver(const Event& e);

        int getNumRects() const;
        int getNumRectsForLayer(LayerId layer) const;

        bool isRectInLayer(Rect2dPtr rec, LayerId layer);

    private:
        Ogre::SceneManager* mSceneManager;
        Ogre::SceneNode* mParentNode;

        std::map<LayerId, TextureSet> mCurrentRects;
    };
}
