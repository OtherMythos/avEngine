#pragma once

#include <memory>

namespace Ogre{
    class Root;
    class SceneNode;
    class SceneManager;
    class Camera;
}

namespace AV {
    class SDL2Window;
    class TestModeManager;
    class Event;
    class ScriptingStateManager;
    class SerialisationManager;
    class ImguiBase;

    class Base{
    public:
        Base();
        /**
         Dependency injection constructor. For use only in tests.
         */
        Base(std::shared_ptr<SDL2Window> window);
        ~Base();

        /**
         Update the base. This will update all the various subsystems.
         */
        void update();

        /**
         @return
         True if the base is running and false if not.
         */
        bool isOpen();

        /**
         Shutdown Base and all its subsystems.
         */
        void shutdown();

        bool testEventReceiver(const Event &e);

    protected:
        /**
         Initialise Base and all its subsystems.
         */
        void _initialise();

    private:
        std::shared_ptr<SDL2Window> _window;
        std::shared_ptr<Ogre::Root> _root;
        std::shared_ptr<TestModeManager> mTestModeManager;
        std::shared_ptr<ScriptingStateManager> mScriptingStateManager;
        std::shared_ptr<ImguiBase> mImguiBase;
        std::shared_ptr<SerialisationManager> mSerialisationManager;
        Ogre::SceneManager* _sceneManager;
        Ogre::Camera* camera;

        bool open = true;

        /**
         Runs the process of setting up ogre.

         This process is abstracted depending on the platform.
         */
        void _setupOgre();
    };
}
