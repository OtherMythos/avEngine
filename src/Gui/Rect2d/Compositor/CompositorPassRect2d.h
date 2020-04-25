#pragma once

#include "OgrePrerequisites.h"
#include "Compositor/Pass/OgreCompositorPass.h"

#include "OgreRenderQueue.h"
#include <memory>

namespace AV{
    class CompositorPassRect2dDef;
    class Rect2dMovable;
    class Rect2dManager;

    /**
    A custom implementation of an ogre scene pass.
    This pass uses a custom rendering queue to define what should be rendered in what order.
    This is used by the texture2d system to render certain textures over others.

    Largely taken from darksylinc/colibrigui
    */
    class CompositorPassRect2d : public Ogre::CompositorPass{
    protected:
        Ogre::SceneManager *mSceneManager;

        Ogre::RenderQueue* mRenderQueue;
        std::shared_ptr<Rect2dManager> mRect2dManager;

    public:
        CompositorPassRect2d(const CompositorPassRect2dDef* definition, Ogre::SceneManager *sceneManager, const Ogre::RenderTargetViewDef* target, Ogre::CompositorNode *parentNode);
        ~CompositorPassRect2d();

        virtual void execute(const Ogre::Camera *lodCamera);
    };
}
