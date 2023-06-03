#include "Base.h"

#include "Logger/Log.h"

#include "Window/SDL2Window/SDL2Window.h"

#include "Scripting/ScriptVM.h"
#include "Scripting/ScriptManager.h"
#include "Scripting/ScriptingStateManager.h"
#include "World/WorldSingleton.h"
#include "Event/Events/SystemEvent.h"
#include "Event/Events/TestingEvent.h"
#include "Event/EventDispatcher.h"
#include "System/SystemSetup/UserSettings.h"

#include "System/TestMode/TestModeManager.h"
#include "Serialisation/SerialisationManager.h"
#include "System/Registry/ValueRegistry.h"
#include "System/Timing/TimerManager.h"

#include "World/Physics/PhysicsShapeManager.h"
#include "World/Physics/PhysicsBodyConstructor.h"
#include "World/Physics/PhysicsBodyDestructor.h"
#include "World/Physics/PhysicsCollisionDataManager.h"
#include "World/Slot/Chunk/TerrainManager.h"
#include "World/Support/OgreMeshManager.h"

#include "Animation/AnimationManager.h"

#ifdef DEBUGGING_TOOLS
    #include "World/Developer/DebugDrawer.h"
#endif

#include "Input/InputManager.h"

#include "Threading/JobDispatcher.h"
#include "Threading/ThreadManager.h"
#include "BaseSingleton.h"

#include "World/Support/ProgrammaticMeshGenerator.h"

#ifdef __APPLE__
    #ifdef TARGET_APPLE_IPHONE
        #include "OgreSetup/iosOgreSetup.h"
    #else
        #include "OgreSetup/MacOSOgreSetup.h"
    #endif
#elif __linux__ || __FreeBSD__
    #include "OgreSetup/LinuxOgreSetup.h"
#elif _WIN32
    #include "OgreSetup/WindowsOgreSetup.h"
#endif

#include "OgreFrameStats.h"

#include "Dialog/Compiler/DialogScriptData.h"
#include "Dialog/DialogManager.h"

#include "Gui/Rect2d/Rect2dManager.h"
#include "Gui/Rect2d/Rect2d.h"
#include "Gui/GuiManager.h"

#include "Audio/OpenAL/AudioManagerOpenAL.h"

namespace AV {

    Base::Base()
        : mScriptingStateManager(std::make_shared<ScriptingStateManager>()),
          mSerialisationManager(std::make_shared<SerialisationManager>()),
          mGuiManager(std::make_shared<GuiManager>()),
          mScriptManager(std::make_shared<ScriptManager>()),
          mTimerManager(std::make_shared<TimerManager>()),
          mAnimationManager(std::make_shared<AnimationManager>()),
          mInputManager(std::make_shared<InputManager>()),
          mAudioManager(std::shared_ptr<AudioManager>(new AudioManagerOpenAL())) {

        _window = std::make_shared<SDL2Window>();

        if(SystemSettings::getPhysicsCompletelyDisabled()) mThreadManager = 0;
        else mThreadManager = std::make_shared<ThreadManager>();

        auto rectMan = std::make_shared<Rect2dManager>();
        Window* win = (Window*)(_window.get());
        BaseSingleton::initialise(
            this,
            win,
            mScriptingStateManager,
            mSerialisationManager,
            std::make_shared<OgreMeshManager>(),
            rectMan,
            std::make_shared<DialogManager>(),
            std::make_shared<ValueRegistry>(),
            std::make_shared<TerrainManager>(),
            mInputManager,
            mTimerManager,
            mGuiManager,
            mScriptManager,
            mAnimationManager,
            mAudioManager
        );
#ifdef DEBUGGING_TOOLS
        BaseSingleton::setupDebuggerTools(
            new DebugDrawer()
        );
#endif

        _initialise();
    }

    Rect2dManager* man;
    Rect2dPtr rec;
    Base::Base(std::shared_ptr<Window>& window)
    : _window(window){

        _initialise();
    }

    Base::~Base(){
        //shutdown();
    }

    void Base::_initialise(){
        JobDispatcher::initialise(SystemSettings::getNumWorkerThreads());
        #ifdef TEST_MODE
            if(SystemSettings::isTestModeEnabled()){
                mTestModeManager = std::make_shared<TestModeManager>();
                mTestModeManager->initialise();
                EventDispatcher::subscribe(EventType::Testing, AV_BIND(Base::testEventReceiver));
            }
        #endif

        ScriptVM::initialise();
        auto inMan = BaseSingleton::getInputManager();
        if(SystemSettings::getUseDefaultActionSet()) inMan->setupDefaultActionSet();
        _window->open(inMan.get(), mGuiManager.get());

        mAudioManager->setup();

        _setupOgre();
        BaseSingleton::getOgreMeshManager()->setupSceneManager(_sceneManager);
        //BaseSingleton::getMovableTextureManager()->initialise(_sceneManager);
        BaseSingleton::getRect2dManager()->initialise(_sceneManager);
        BaseSingleton::getDialogManager()->initialise();

        ProgrammaticMeshGenerator::createMesh();
        PhysicsBodyConstructor::setup();
        PhysicsBodyDestructor::setup();
        PhysicsCollisionDataManager::startup();

        //TODO This can be done with some sort of startup event where pointers are broadcast, rather than manually.
        ScriptVM::injectPointers(camera, _sceneManager, mScriptingStateManager.get());

        mScriptingStateManager->initialise();

        #ifdef DEBUGGING_TOOLS
            BaseSingleton::getDebugDrawer()->initialise(_sceneManager);
        #endif

        if(UserSettings::getDeveloperModeGuiEnabled()){
            mGuiManager->showDebugMenu(true);
        }
    }

#ifdef TEST_MODE
    bool Base::testEventReceiver(const Event &e){
        const TestingEvent& testEvent = (TestingEvent&)e;
        if(testEvent.eventId() == EventId::TestingBooleanAssertFailed
            || testEvent.eventId() == EventId::TestingComparisonAssertFailed
            || testEvent.eventId() == EventId::TestingTestEnd
            || testEvent.eventId() == EventId::TestingScriptFailure
            || testEvent.eventId() == EventId::TestingTimeoutReached){
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
        static Ogre::Timer timer;
        static Ogre::uint64 startTime = timer.getMicroseconds();
        static double timeSinceLast = 1.0 / 60.0;

        _window->update();

        PhysicsBodyDestructor::update();

        //Queue the threads to start processing for this frame.
        if(mThreadManager) mThreadManager->sheduleUpdate(1);

#ifdef TEST_MODE
        if(SystemSettings::isTestModeEnabled()){
            mTestModeManager->updateTimeout();
        }
#endif

#ifdef DEBUGGING_TOOLS
        BaseSingleton::getDebugDrawer()->resetDraw();
#endif

        mScriptingStateManager->updateBaseState();

        World* w = WorldSingleton::getWorldNoCheck();
        if(w){
            w->update();
        }

        mScriptingStateManager->update();
        mScriptManager->processEvents();
        mTimerManager->update(1);
        mAnimationManager->update();
        mInputManager->update(0.01f);
        BaseSingleton::mDialogManager->update();

        mGuiManager->update(60.0f/1000.0f);

        _root->renderOneFrame();


        Ogre::uint64 endTime = timer.getMicroseconds();
        timeSinceLast = (endTime - startTime) / 1000000.0;
        timeSinceLast = std::min( 1.0, timeSinceLast );
        BaseSingleton::mPerformanceStats.frameTime = timeSinceLast;
        const Ogre::FrameStats* frameStats = _root->getFrameStats();
        BaseSingleton::mPerformanceStats.avgFPS = frameStats->getAvgFps();
        BaseSingleton::mPerformanceStats.fps = frameStats->getFps();
        startTime = endTime;
    }

    bool Base::isOpen(){
        //return _window->isOpen();
        return open;
    }

    void Base::_setupOgre(){
        #ifdef __APPLE__
            #ifdef TARGET_APPLE_IPHONE
                iosOgreSetup setup;
            #else
                MacOSOgreSetup setup;
            #endif
        #elif __linux__ || __FreeBSD__
        LinuxOgreSetup setup;
        #elif _WIN32
        WindowsOgreSetup setup;
        #endif

        setup.setupLogger();
        Ogre::Root *root = setup.setupRoot();
        //_root = std::shared_ptr<Ogre::Root>(root);
        _root = root;


        setup.setupOgreWindow((Window*)_window.get());
        mGuiManager->setupColibriManager();
        mGuiManager->setupCompositorProvider(root->getCompositorManager2());
        setup.setupOgreResources(root);
        setup.setupHLMS(root);

        Ogre::SceneManager *sceneManager;
        setup.setupScene(root, &sceneManager, &camera);

        setup.setupCompositor(root, sceneManager, camera, _window->getRenderWindow());
        _sceneManager = sceneManager;
        //_sceneManager = std::shared_ptr<Ogre::SceneManager>(sceneManager);

        mGuiManager->setup(root, sceneManager);
    }

    void Base::shutdown(){
        SystemEventEngineClose closeEvent;
        EventDispatcher::transmitEvent(EventType::System, closeEvent);

        BaseSingleton::getDialogManager()->shutdown();

        WorldSingleton::destroyWorld();
        mScriptingStateManager->shutdown();
        mGuiManager->shutdown();
        PhysicsCollisionDataManager::shutdown();
        ScriptVM::shutdown();
        JobDispatcher::shutdown();
        PhysicsBodyConstructor::shutdown();
        PhysicsBodyDestructor::shutdown();
        PhysicsShapeManager::shutdown();
        ProgrammaticMeshGenerator::shutdown();
        //_root->shutdown();

        #ifdef DEBUGGING_TOOLS
            delete BaseSingleton::getDebugDrawer();
        #endif

        _window->close();
        mAudioManager->shutdown();
        delete _root;
        open = false;
    }
}
