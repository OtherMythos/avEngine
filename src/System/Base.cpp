#include "Base.h"

#include "Logger/Log.h"
#include "Window/SDL2Window/SDL2Window.h"
#include "Scripting/ScriptManager.h"
#include "World/WorldSingleton.h"
#include "Event/Events/SystemEvent.h"
#include "Event/Events/TestingEvent.h"
#include "Event/EventDispatcher.h"

#include "System/TestMode/TestModeManager.h"

#include "Threading/JobDispatcher.h"

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
        ScriptManager::injectPointers(camera, _sceneManager);
        //Run the startup script.
        ScriptManager::runScript(SystemSettings::getSquirrelEntryScriptPath());
    }

    bool Base::testEventReceiver(const Event &e){
        const TestingEvent& testEvent = (TestingEvent&)e;
        if(testEvent.eventCategory() == TestingEventCategory::booleanAssertFailed
            || testEvent.eventCategory() == TestingEventCategory::comparisonAssertFailed
            || testEvent.eventCategory() == TestingEventCategory::testEnd
            || testEvent.eventCategory() == TestingEventCategory::scriptFailure){
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

        World* w = WorldSingleton::getWorld();
        if(w){
          w->update(camera);
        }

        ScriptManager::callFunction(SystemSettings::getSquirrelEntryScriptPath(), "update");

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

        ScriptManager::shutdown();
        JobDispatcher::shutdown();
        _root->shutdown();
        _window->close();
        open = false;
    }
}
