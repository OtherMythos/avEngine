#include "Base.h"

#include "Logger/Log.h"
#include "Window/SDL2Window/SDL2Window.h"
#include "Scripting/ScriptManager.h"
#include "Scripting/ScriptingStateManager.h"
#include "World/WorldSingleton.h"
#include "Event/Events/SystemEvent.h"
#include "Event/Events/TestingEvent.h"
#include "Event/EventDispatcher.h"

#include "System/TestMode/TestModeManager.h"
#include "Serialisation/SerialisationManager.h"

#include "Threading/JobDispatcher.h"
#include "Threading/ThreadManager.h"
#include "BaseSingleton.h"

#include "Gui/Developer/ImguiBase.h"

#ifdef __APPLE__
    #include "OgreSetup/MacOSOgreSetup.h"
#elif __linux__
    #include "OgreSetup/LinuxOgreSetup.h"
#elif _WIN32
	#include "OgreSetup/WindowsOgreSetup.h"
#endif


namespace AV {
    Base::Base(){
        _window = std::make_shared<SDL2Window>();
        mScriptingStateManager = std::make_shared<ScriptingStateManager>();
        mSerialisationManager = std::make_shared<SerialisationManager>();
        mImguiBase = std::make_shared<ImguiBase>();
        mThreadManager = std::make_shared<ThreadManager>();

        BaseSingleton::initialise(
            mScriptingStateManager,
            mSerialisationManager
        );

        _initialise();
    }

    Base::Base(std::shared_ptr<SDL2Window> window)
    : _window(window){

        _initialise();
    }

    Base::~Base(){
        //shutdown();
    }

    void Base::_initialise(){
        JobDispatcher::initialise(4);
        if(SystemSettings::isTestModeEnabled()){
            mTestModeManager = std::make_shared<TestModeManager>();
            mTestModeManager->initialise();
            EventDispatcher::subscribe(EventType::Testing, AV_BIND(Base::testEventReceiver));
        }

        ScriptManager::initialise();
        _window->open();

        _setupOgre();

        //TODO This can be done with some sort of startup event where pointers are broadcast, rather than manually.
        ScriptManager::injectPointers(camera, _sceneManager, mScriptingStateManager.get());

        mImguiBase->initialise(_sceneManager);
    }

    bool Base::testEventReceiver(const Event &e){
        const TestingEvent& testEvent = (TestingEvent&)e;
        if(testEvent.eventCategory() == TestingEventCategory::booleanAssertFailed
            || testEvent.eventCategory() == TestingEventCategory::comparisonAssertFailed
            || testEvent.eventCategory() == TestingEventCategory::testEnd
            || testEvent.eventCategory() == TestingEventCategory::scriptFailure
            || testEvent.eventCategory() == TestingEventCategory::timeoutReached){
            //Close the engine down if the test fails an assertion, or if the test should end.
            open = false;
        }
        return true;
    }

    void Base::update(){
        if(_window->wantsToClose){
            //shutdown();
            open = false;
            return;
        }
        Ogre::WindowEventUtilities::messagePump();
        _window->update();

        //As a possible optimisation this could be moved somewhere else at a later date, so less ifs in the critical path.
        //I have no idea where else it would go though, other than a pre-processor macro :(
        if(SystemSettings::isTestModeEnabled()){
            mTestModeManager->updateTimeout();
        }

        World* w = WorldSingleton::getWorldNoCheck();
        if(w){
            w->update();
        }

        mScriptingStateManager->update();

        _root->renderOneFrame();
    }

    bool Base::isOpen(){
        //return _window->isOpen();
        return open;
    }

    void Base::_setupOgre(){
        #ifdef __APPLE__
        MacOSOgreSetup setup;
        #elif __linux__
        LinuxOgreSetup setup;
    		#elif _WIN32
    		WindowsOgreSetup setup;
        #endif

        Ogre::Root *root = setup.setupRoot();
        _root = std::shared_ptr<Ogre::Root>(root);

        setup.setupOgreWindow((Window*)_window.get());
        setup.setupOgreResources(root);
        setup.setupHLMS(root);

        Ogre::SceneManager *sceneManager;
        setup.setupScene(root, &sceneManager, &camera);
        setup.setupCompositor(root, sceneManager, camera, _window->getRenderWindow());
        _sceneManager = sceneManager;
        //_sceneManager = std::shared_ptr<Ogre::SceneManager>(sceneManager);
    }

    void Base::shutdown(){
        SystemEventEngineClose closeEvent;
        EventDispatcher::transmitEvent(EventType::System, closeEvent);

        WorldSingleton::destroyWorld();
        mScriptingStateManager->shutdown();
        ScriptManager::shutdown();
        JobDispatcher::shutdown();
        _root->shutdown();
        _window->close();
        open = false;
    }
}
