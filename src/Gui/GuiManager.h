#pragma once

#include "OgreVector2.h"

namespace Ogre{
    class Root;
    class SceneManager;
    class CompositorManager2;
}

namespace Colibri{
    class ColibriManager;
    class Window;
    class Label;
    class LayoutLine;
}

namespace AV{
    class Event;

    /**
    A class responsible for managing colibri gui.
    This includes things like startup and shutdown, as well as access to the necessary components.
    */
    class GuiManager{
    private:
        float mScrollSpeed = 1.0f;
        Ogre::Vector2 mGuiMousePos;

    public:
        GuiManager();
        ~GuiManager();

        void setup(Ogre::Root* root, Ogre::SceneManager* sceneManager);
        void update(float timeSinceLast);
        void shutdown();

        void showDebugMenu(bool show);
        void toggleDebugMenu(){ showDebugMenu(!mDebugVisible); }

        void setMouseScrollSpeed(float speed) { mScrollSpeed = speed; };
        float getMouseScrollSpeed() const { return mScrollSpeed; }

        uint32_t getOriginalDefaultFontSize26d6() const { return mOriginalDefaultFontSize; }

        void setupCompositorProvider(Ogre::CompositorManager2* compMan);
        void setupColibriManager();

        void reprocessMousePosition();
        void setGuiMousePos(const Ogre::Vector2& vec);

        const Ogre::Vector2& getGuiMousePos() const { return mGuiMousePos; }

        Colibri::ColibriManager* getColibriManager() const { return mColibriManager; }

    private:
        Colibri::ColibriManager* mColibriManager = 0;
        uint32_t mOriginalDefaultFontSize = 0;

        //Load and create the resources for the default skin. This should be called if no skins were supplied, by the user.
        void _loadDefaultSkin();

        bool debuggerToolsReceiver(const Event &e);

        bool mDebugVisible = false;
        bool mDebugMenuSetup = false;
        Colibri::Window* mDebugWindow = 0;
        static const int numDebugWindowLabels = 6;
        Colibri::Label* mDebugWindowLabels[numDebugWindowLabels];
        Colibri::LayoutLine* mDebugWindowLabel = 0;
        void _constructDebugWindow();

        void _updateDebugMenuText();
    };
}
