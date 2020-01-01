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

#include "World/Physics/PhysicsShapeManager.h"
#include "World/Physics/PhysicsBodyConstructor.h"
#include "World/Physics/PhysicsBodyDestructor.h"
#include "World/Support/OgreMeshManager.h"

#include "Threading/JobDispatcher.h"
#include "Threading/ThreadManager.h"
#include "BaseSingleton.h"

#include "Gui/Texture2d/MovableTextureManager.h"

#include "World/Support/ProgrammaticMeshGenerator.h"

#ifdef DEBUGGING_TOOLS
    #include "Gui/Developer/ImguiBase.h"
#endif

#ifdef __APPLE__
    #include "OgreSetup/MacOSOgreSetup.h"
#elif __linux__
    #include "OgreSetup/LinuxOgreSetup.h"
#elif _WIN32
    #include "OgreSetup/WindowsOgreSetup.h"
#endif

#include "Dialog/Compiler/DialogScriptData.h"
#include "Dialog/DialogManager.h"

#include "Gui/Rect2d/Rect2dManager.h"
#include "Gui/Rect2d/Rect2d.h"

namespace AV {
    Base::Base()
        : _window(std::make_shared<SDL2Window>()),
          mScriptingStateManager(std::make_shared<ScriptingStateManager>()),
          mSerialisationManager(std::make_shared<SerialisationManager>()),
          #ifdef DEBUGGING_TOOLS
            mImguiBase(std::make_shared<ImguiBase>()),
          #endif
          mThreadManager(std::make_shared<ThreadManager>()){

        Window* win = (Window*)(_window.get());
        BaseSingleton::initialise(
            win,
            mScriptingStateManager,
            mSerialisationManager,
            std::make_shared<OgreMeshManager>(),
            std::make_shared<MovableTextureManager>(),
            std::make_shared<Rect2dManager>(),
            std::make_shared<DialogManager>()
        );

        _initialise();
    }

    Rect2dManager* man;
    Rect2dPtr rec;
    Base::Base(std::shared_ptr<SDL2Window>& window)
    : _window(window){

        _initialise();
    }

    Base::~Base(){
        //shutdown();
    }

    void Base::_initialise(){
        JobDispatcher::initialise(4);
        #ifdef TEST_MODE
            if(SystemSettings::isTestModeEnabled()){
                mTestModeManager = std::make_shared<TestModeManager>();
                mTestModeManager->initialise();
                EventDispatcher::subscribe(EventType::Testing, AV_BIND(Base::testEventReceiver));
            }
        #endif

        ScriptManager::initialise();
        _window->open();

        _setupOgre();
        BaseSingleton::getOgreMeshManager()->setupSceneManager(_sceneManager);
        BaseSingleton::getMovableTextureManager()->initialise(_sceneManager);
        BaseSingleton::getRect2dManager()->initialise(_sceneManager);
        BaseSingleton::getDialogManager()->initialise();

        ProgrammaticMeshGenerator::createMesh();
        PhysicsBodyConstructor::setup();
        PhysicsBodyDestructor::setup();

        //TODO This can be done with some sort of startup event where pointers are broadcast, rather than manually.
        ScriptManager::injectPointers(camera, _sceneManager, mScriptingStateManager.get());

        #ifdef DEBUGGING_TOOLS
            mImguiBase->initialise(_sceneManager);
        #endif
        mScriptingStateManager->initialise();

    }

#ifdef TEST_MODE
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
#endif

    void Base::update(){
        if(_window->wantsToClose){
            //shutdown();
            open = false;
            return;
        }
        _window->update();

        PhysicsBodyDestructor::update();

        //Queue the threads to start processing for this frame.
        mThreadManager->sheduleUpdate(1);

#ifdef TEST_MODE
        if(SystemSettings::isTestModeEnabled()){
            mTestModeManager->updateTimeout();
        }
#endif

        World* w = WorldSingleton::getWorldNoCheck();
        if(w){
            w->update();
        }

        mScriptingStateManager->update();
        BaseSingleton::mDialogManager->update();

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
        PhysicsBodyConstructor::shutdown();
        PhysicsBodyDestructor::shutdown();
        PhysicsShapeManager::shutdown();
        _root->shutdown();
        _window->close();
        open = false;
    }
}
