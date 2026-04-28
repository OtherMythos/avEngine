#include "Base.h"

#include "Logger/Log.h"

#include <chrono>
#include <thread>

#include "Util/SimpleFileParser.h"

#include "Window/SDL2Window/SDL2Window.h"

#include "Scripting/ScriptVM.h"
#include "Scripting/ScriptManager.h"
#include "Scripting/ScriptingStateManager.h"
#include "World/WorldSingleton.h"
#include "Event/Events/SystemEvent.h"
#include "Event/Events/TestingEvent.h"
#include "Event/EventDispatcher.h"
#include "System/SystemSetup/UserSettings.h"
#include "System/Plugins/PluginManager.h"

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

#include "unicode/ucnv.h"

#include "Animation/AnimationManager.h"

#ifdef DEBUGGING_TOOLS
    #include "World/Developer/DebugDrawer.h"
#endif

#include "Input/InputManager.h"

#include "Threading/JobDispatcher.h"
#include "Threading/ThreadManager.h"
#include "BaseSingleton.h"

#include "World/Support/ProgrammaticMeshGenerator.h"
#include "World/Support/InternalTextureManager.h"

#ifdef __APPLE__
    #ifdef TARGET_APPLE_IPHONE
        #include "OgreSetup/iosOgreSetup.h"
    #else
        #include "OgreSetup/MacOSOgreSetup.h"
    #endif
#elif defined(TARGET_ANDROID)
    #include "OgreSetup/AndroidOgreSetup.h"
#elif (defined(__linux__) || defined(__FreeBSD__)) && !defined(TARGET_ANDROID)
    #include "OgreSetup/LinuxOgreSetup.h"
#elif _WIN32
    #include "OgreSetup/WindowsOgreSetup.h"
#endif

#ifdef ENABLE_ADMOB
    #include "Advertising/AdManager.h"
    #ifdef TARGET_APPLE_IPHONE
        #include "Advertising/iOS/iosAdManager.h"
    #else
        #include "Advertising/AdManagerNull.h"
    #endif
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
          mGuiInputProcessor(std::make_shared<GuiInputProcessor>()),
          mAudioManager(std::shared_ptr<AudioManager>(new AudioManagerOpenAL())) {

        ucnv_setDefaultName("UTF-8");

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
            mGuiInputProcessor,
            mScriptManager,
            mAnimationManager,
            mAudioManager
        );
#ifdef DEBUGGING_TOOLS
        BaseSingleton::setupDebuggerTools(
            new DebugDrawer()
        );
#endif

#ifdef ENABLE_ADMOB
        #ifdef TARGET_APPLE_IPHONE
            mAdManager = std::make_unique<iosAdManager>();
        #else
            mAdManager = std::make_unique<AdManagerNull>();
        #endif
        AdManager::setInstance(mAdManager.get());
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
        _window->initialise();
        #ifdef TEST_MODE
            if(SystemSettings::isTestModeEnabled()){
                mTestModeManager = std::make_shared<TestModeManager>();
                mTestModeManager->initialise();
                EventDispatcher::subscribe(EventType::Testing, AV_BIND(Base::testEventReceiver));
            }
        #endif

        ScriptVM::initialise(SystemSettings::getUseSetupFunction());
        if(SystemSettings::getUseDefaultActionSet()) mInputManager->setupDefaultActionSet();
        _window->open(mInputManager.get(), mGuiInputProcessor.get());

        mAudioManager->setup();

        #ifdef ENABLE_ADMOB
            if(mAdManager) mAdManager->initialise();
        #endif

        _setupOgre();
        BaseSingleton::getOgreMeshManager()->setupSceneManager(_sceneManager);
        //BaseSingleton::getMovableTextureManager()->initialise(_sceneManager);
        BaseSingleton::getRect2dManager()->initialise(_sceneManager);
        BaseSingleton::getDialogManager()->initialise();

        ProgrammaticMeshGenerator::createMesh();
        InternalTextureManager::createTextures();
        PhysicsBodyConstructor::setup();
        PhysicsBodyDestructor::setup();
        PhysicsCollisionDataManager::startup();

        //TODO This can be done with some sort of startup event where pointers are broadcast, rather than manually.
        ScriptVM::injectPointers(camera, _sceneManager, mScriptingStateManager.get());
        BaseSingleton::mSceneManager = _sceneManager;

        mScriptingStateManager->initialise();

        #ifdef DEBUGGING_TOOLS
            BaseSingleton::getDebugDrawer()->initialise(_sceneManager);
        #endif

        if(UserSettings::getDeveloperModeGuiEnabled()){
            mGuiManager->showDebugMenu(true);
        }

        PluginManager::initialise();
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
            open = false;
            return;
        }

        static Ogre::Timer timer;
        static Ogre::uint64 lastTime = timer.getMicroseconds();
        static double accumulator = 0.0;
        static const double fixedDeltaTime = 1.0 / static_cast<double>(SystemSettings::getFixedUpdateRate());
        static const double maxElapsed = 0.25;
        static const int maxSteps = 4;

        //Measure time elapsed since the last frame.
        //This naturally accounts for vsync blocking inside renderOneFrame.
        Ogre::uint64 now = timer.getMicroseconds();
        double elapsed = static_cast<double>(now - lastTime) / 1000000.0;
        lastTime = now;

        //Clamp elapsed time to prevent spiral of death on slow machines or after breakpoints.
        if(elapsed > maxElapsed) elapsed = maxElapsed;

        accumulator += elapsed;

        //Per-frame work that should only run once regardless of fixed step count.

        //This must be called before anything else so the scene can be guaranteed clean.
        mScriptingStateManager->updateBaseState();

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

        //Fixed timestep game logic loop.
        float fixedDt = static_cast<float>(fixedDeltaTime);
        mScriptingStateManager->setFixedDeltaTime(fixedDt);
        int steps = 0;
        while(accumulator >= fixedDeltaTime && steps < maxSteps){
            World* w = WorldSingleton::getWorldNoCheck();
            if(w){
                w->update();
            }

            mScriptingStateManager->update();
            mScriptManager->processEvents();
            mTimerManager->update(static_cast<uint64>(fixedDeltaTime * 1000.0));
            mAnimationManager->update();
            mInputManager->update(fixedDt);
            BaseSingleton::mDialogManager->update();

            mGuiManager->update(fixedDt);

            accumulator -= fixedDeltaTime;
            steps++;
        }

        //Flush any dirty labels from input events that arrived this frame but weren't
        //processed by a fixed-step update (happens when fps > update rate).
        mGuiManager->flushDirtyLabels();

        //Render once per frame. With vsync enabled this may block until the next display refresh.
        _root->renderOneFrame();

        const Ogre::FrameStats* frameStats = _root->getFrameStats();
        BaseSingleton::mPerformanceStats.frameTime = frameStats->getLastTime() / 1000.0f;
        BaseSingleton::mPerformanceStats.avgFPS = frameStats->getAvgFps();
        BaseSingleton::mPerformanceStats.fps = frameStats->getFps();
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
        #elif defined(TARGET_ANDROID)
            AndroidOgreSetup setup;
        #elif (defined(__linux__) || defined(__FreeBSD__)) && !defined(TARGET_ANDROID)
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
        mGuiInputProcessor->initialise(mGuiManager.get());
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
        mAudioManager->shutdown();
        mAnimationManager->shutdown();
        #ifdef DEBUGGING_TOOLS
            delete BaseSingleton::getDebugDrawer();
        #endif

        PluginManager::shutdown();
        delete _root;
        _window->close();
        PluginManager::unload();
        open = false;
    }
}
