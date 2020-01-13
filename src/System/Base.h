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
    class ThreadManager;
    class PhysicsBodyDestructor;

    class Base{
    public:
        Base();
        /**
         Dependency injection constructor. For use only in tests.
         */
        Base(std::shared_ptr<SDL2Window>& window);
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

        #ifdef TEST_MODE
            bool testEventReceiver(const Event &e);
        #endif

    protected:
        /**
         Initialise Base and all its subsystems.
         */
        void _initialise();

    private:
        std::shared_ptr<SDL2Window> _window;
        //std::shared_ptr<Ogre::Root> _root;
        Ogre::Root* _root;
        #ifdef TEST_MODE
            std::shared_ptr<TestModeManager> mTestModeManager;
        #endif
        #ifdef DEBUGGING_TOOLS
            std::shared_ptr<ImguiBase> mImguiBase;
        #endif
        std::shared_ptr<ScriptingStateManager> mScriptingStateManager;
        std::shared_ptr<SerialisationManager> mSerialisationManager;
        std::shared_ptr<PhysicsBodyDestructor> mBodyDestructor;
        std::shared_ptr<ThreadManager> mThreadManager;
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
