#pragma once

#include <OgreFrameListener.h>

namespace Ogre{
    class SceneManager;
}

namespace AV{
    class ImguiBase : public Ogre::FrameListener{
    public:
        ImguiBase();

        void initialise(Ogre::SceneManager *sceneManager);

    private:
        void _setupImguiStyle();

        bool frameRenderingQueued(const Ogre::FrameEvent& evt);

        void processInput();
    };
}
