#pragma once

namespace Ogre{
    class Root;
    class SceneManager;
    class CompositorManager2;
}

namespace Colibri{
    class ColibriManager;
}

namespace AV{

    /**
    A class responsible for managing colibri gui.
    This includes things like startup and shutdown, as well as access to the necessary components.
    */
    class GuiManager{
    public:
        GuiManager();
        ~GuiManager();

        void setup(Ogre::Root* root, Ogre::SceneManager* sceneManager);
        void update(float timeSinceLast);

        void setupCompositorProvider(Ogre::CompositorManager2* compMan);
        void setupColibriManager();

    private:
        Colibri::ColibriManager* mColibriManager = 0;
    };
}
