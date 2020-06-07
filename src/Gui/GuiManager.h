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
        void shutdown();

        void setupCompositorProvider(Ogre::CompositorManager2* compMan);
        void setupColibriManager();

        Colibri::ColibriManager* getColibriManager() const { return mColibriManager; }

    private:
        Colibri::ColibriManager* mColibriManager = 0;

        //Load and create the resources for the default skin. This should be called if no skins were supplied, by the user.
        void _loadDefaultSkin();
    };
}
