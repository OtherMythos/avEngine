#include "Base.h"

#include "Logger/Log.h"

#include <chrono>
#include <thread>

#include "Util/SimpleFileParser.h"

#include "Window/SDL2Window/SDL2Window.h"
#if !defined(TARGET_APPLE_IPHONE) && !defined(TARGET_ANDROID)
    #include "Window/HeadlessWindow/HeadlessWindow.h"
#endif

#include "Scripting/ScriptVM.h"
#include "Scripting/Worker/ScriptWorkerManager.h"
#include "Scripting/ScriptManager.h"
#include "Scripting/ScriptingStateManager.h"
#include "Scripting/ScriptPluginManager.h"
#include "Event/Events/SystemEvent.h"
#include "Event/Events/TestingEvent.h"
#include "Event/EventDispatcher.h"
#include "System/Plugins/PluginManager.h"

#include "System/TestMode/TestModeManager.h"
#include "System/Registry/ValueRegistry.h"

#include "Physics/PhysicsShapeManager.h"
#include "Physics/PhysicsBodyConstructor.h"
#include "Physics/PhysicsBodyDestructor.h"
#include "Physics/PhysicsCollisionDataManager.h"
#include "Entity/EntityManager.h"
#include "Physics/PhysicsManager.h"
#include "Mesh/OgreMeshManager.h"

#include "unicode/ucnv.h"

#include "Animation/AnimationManager.h"

#ifdef DEBUGGING_TOOLS
    #include "Developer/DebugDrawer.h"
    #include "Developer/MeshVisualiser.h"
#endif

#ifdef DEBUG_SERVER
    #include "System/DebugServer/DebugServer.h"
#endif

#ifdef SCRIPT_PROFILER
    #include "Scripting/Profiler/ScriptProfiler.h"
#endif

#include "Input/InputManager.h"
#include "Input/InputRouter.h"

#include "Threading/JobDispatcher.h"
#include "Threading/ThreadManager.h"
#include "BaseSingleton.h"

#include "Mesh/ProgrammaticMeshGenerator.h"
#include "Mesh/InternalTextureManager.h"

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

#include "DataStore/DataStore.h"
#ifdef TARGET_APPLE_IPHONE
    #include "DataStore/iOS/iosDataStore.h"
#else
    #include "DataStore/DataStoreNull.h"
#endif

#ifdef ENABLE_ADMOB
    #include "Advertising/AdManager.h"
    #ifdef TARGET_APPLE_IPHONE
        #include "Advertising/iOS/iosAdManager.h"
    #else
        #include "Advertising/AdManagerNull.h"
    #endif
#endif

#ifdef ENABLE_MICROTRANSACTIONS
    #include "Microtransaction/PurchaseManager.h"
    #ifdef TARGET_APPLE_IPHONE
        #include "Microtransaction/iOS/iosPurchaseManager.h"
    #else
        #include "Microtransaction/PurchaseManagerNull.h"
    #endif
#endif



#include "Gui/Rect2d/Rect2dManager.h"
#include "Gui/Rect2d/Rect2d.h"
#include "Gui/GuiManager.h"

#include "Audio/OpenAL/AudioManagerOpenAL.h"

namespace AV {

    Base::Base()
        : mScriptingStateManager(std::make_shared<ScriptingStateManager>()),
          mScriptPluginManager(std::make_shared<ScriptPluginManager>()),
          mGuiManager(std::make_shared<GuiManager>()),
          mScriptManager(std::make_shared<ScriptManager>()),
          mAnimationManager(std::make_shared<AnimationManager>()),
          mInputManager(std::make_shared<InputManager>()),
          mInputRouter(std::make_shared<InputRouter>()),
          mGuiInputProcessor(std::make_shared<GuiInputProcessor>()),
          mAudioManager(std::shared_ptr<AudioManager>(new AudioManagerOpenAL())) {

        ucnv_setDefaultName("UTF-8");

#if !defined(TARGET_APPLE_IPHONE) && !defined(TARGET_ANDROID)
        if(SystemSettings::isHeadless()){
            _window = std::make_shared<HeadlessWindow>();
        }else
#endif
        {
            _window = std::make_shared<SDL2Window>();
        }

        if(SystemSettings::getPhysicsCompletelyDisabled()) mThreadManager = 0;
        else mThreadManager = std::make_shared<ThreadManager>();

        auto rectMan = std::make_shared<Rect2dManager>();
        Window* win = (Window*)(_window.get());
        BaseSingleton::initialise(
            this,
            win,
            mScriptingStateManager,
            std::make_shared<OgreMeshManager>(),
            rectMan,
            std::make_shared<ValueRegistry>(),
            mInputManager,
            mInputRouter,
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

#ifdef TARGET_APPLE_IPHONE
        mDataStore = std::make_unique<iosDataStore>();
#else
        mDataStore = std::make_unique<DataStoreNull>();
#endif
        DataStore::setInstance(mDataStore.get());

#ifdef ENABLE_ADMOB
        #ifdef TARGET_APPLE_IPHONE
            mAdManager = std::make_unique<iosAdManager>();
        #else
            mAdManager = std::make_unique<AdManagerNull>();
        #endif
        AdManager::setInstance(mAdManager.get());
#endif

#ifdef ENABLE_MICROTRANSACTIONS
        #ifdef TARGET_APPLE_IPHONE
            mPurchaseManager = std::make_unique<iosPurchaseManager>();
        #else
            mPurchaseManager = std::make_unique<PurchaseManagerNull>();
        #endif
        PurchaseManager::setInstance(mPurchaseManager.get());
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
        mSimulationClock = SimulationClock(SystemSettings::getFixedUpdateRate());
        _window->initialise();
        #ifdef TEST_MODE
            if(SystemSettings::isTestModeEnabled()){
                mTestModeManager = std::make_shared<TestModeManager>();
                mTestModeManager->initialise();
                EventDispatcher::subscribe(EventType::Testing, AV_BIND(Base::testEventReceiver));
            }
        #endif

        ScriptVM::initialise(SystemSettings::getUseSetupFunction());
        //Only setup the namespace if the script worker is enabled.
        if(SystemSettings::getScriptWorkersEnabled()){
            mScriptWorkerManager = std::make_shared<ScriptWorkerManager>();
        }

        if(SystemSettings::getUseDefaultActionSet()) mInputManager->setupDefaultActionSet();
        //Before the window opens, so anything the window dispatches during open
        //already has a layer stack to go through.
        mInputRouter->initialise(mInputManager.get(), mGuiInputProcessor.get(), _window.get());
        _window->open(mInputManager.get(), mGuiInputProcessor.get(), mInputRouter.get());

        mAudioManager->setup();

        #ifdef ENABLE_ADMOB
            if(mAdManager) mAdManager->initialise();
        #endif
        #ifdef ENABLE_MICROTRANSACTIONS
            if(mPurchaseManager) mPurchaseManager->initialise();
        #endif

        _setupOgre();
        BaseSingleton::getOgreMeshManager()->setupSceneManager(_sceneManager);
        //BaseSingleton::getMovableTextureManager()->initialise(_sceneManager);
        BaseSingleton::getRect2dManager()->initialise(_sceneManager);

        ProgrammaticMeshGenerator::createMesh();
        InternalTextureManager::createTextures();

        PhysicsBodyConstructor::setup();
        PhysicsBodyDestructor::setup();
        PhysicsCollisionDataManager::startup();

        //These must come after the static physics setup above.
        //Constructing the PhysicsManager creates the collision worlds, which allocate out of
        //the data packers that setup()/startup() clear.
        mEntityManager = std::make_shared<EntityManager>();
        mEntityManager->initialise();
        mPhysicsManager = std::make_shared<PhysicsManager>();
        mEntityManager->setPhysicsManager(mPhysicsManager);

        if(mThreadManager) mThreadManager->notifyPhysicsManagerCreated(mPhysicsManager);

        BaseSingleton::mEntityManager = mEntityManager;
        BaseSingleton::mPhysicsManager = mPhysicsManager;

        #ifdef DEBUGGING_TOOLS
            mMeshVisualiser = std::make_shared<MeshVisualiser>();
            mMeshVisualiser->initialise(_sceneManager);
            BaseSingleton::mMeshVisualiser = mMeshVisualiser;
        #endif

        //TODO This can be done with some sort of startup event where pointers are broadcast, rather than manually.
        ScriptVM::injectPointers(camera, _sceneManager, mScriptingStateManager.get());
        BaseSingleton::mSceneManager = _sceneManager;

        mScriptingStateManager->initialise();

        #ifdef DEBUGGING_TOOLS
            BaseSingleton::getDebugDrawer()->initialise(_sceneManager);
        #endif

        PluginManager::initialise();
        //After the native plugins, so a plugin's script can use a namespace its own native half
        //registered. The entry script's start() doesn't run until the first update, so plugins
        //are ready before any project code.
        mScriptPluginManager->initialise();

        EventDispatcher::subscribe(EventType::System, AV_BIND(Base::systemEventReceiver));

        #ifdef DEBUG_SERVER
            if(SystemSettings::isDebugServerEnabled()){
                mDebugServer = std::make_shared<DebugServer>();
                if(!mDebugServer->initialise(SystemSettings::getDebugServerPort())){
                    AV_ERROR("Engine startup aborted because --debugServer requires exclusive ownership of its port.");
                    mDebugServer.reset();
                    mExitCode = 1;
                    open = false;
                }
            }
        #endif
    }

    bool Base::systemEventReceiver(const Event &e){
        const SystemEvent& sysEvent = static_cast<const SystemEvent&>(e);
        switch(sysEvent.eventId()){
            case EventId::SystemApplicationWillEnterBackground:
            case EventId::SystemApplicationDidEnterBackground:
                mInBackground = true;
                break;
            case EventId::SystemApplicationWillEnterForeground:
                break;
            case EventId::SystemApplicationDidEnterForeground:
                mInBackground = false;
                if(mGuiManager){
                    mGuiManager->notifyEnterForeground();
                }
                break;
            default:
                break;
        }
        return true;
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

    void Base::pauseSimulation(const std::string& reason){
        mSimulationClock.pause(reason);
        if(!mSimulationClock.inTick()) mAudioManager->setSimulationPaused(true);
    }

    bool Base::resumeSimulation(double speed){
        if(!mSimulationClock.resume(speed)) return false;
        mAudioManager->setSimulationPaused(false);
        return true;
    }

    void Base::update(){
        if(!open) return;
        if(_window->wantsToClose){
            open = false;
            return;
        }

        const auto now = std::chrono::steady_clock::now();
        const double wallSeconds = std::chrono::duration<double>(now - mLastUpdate).count();
        mSimulationClock.accumulate(wallSeconds);
        mLastUpdate = now;

        //Window and developer UI remain responsive even when no simulation ticks are granted.
        PluginManager::frameUpdate();
        _window->update();
#ifdef TEST_MODE
        if(SystemSettings::isTestModeEnabled()) mTestModeManager->updateTimeout();
#endif

        const float fixedDt = static_cast<float>(mSimulationClock.deltaSeconds());
        mScriptingStateManager->setFixedDeltaTime(fixedDt);
        mScriptPluginManager->setFixedDeltaTime(fixedDt);
        int maxSteps = mSimulationClock.mode() == SimulationClock::Mode::STEPPING ? 1 : 4;
#ifdef DEBUG_SERVER
        if(mDebugServer && mSimulationClock.mode() == SimulationClock::Mode::RUNNING){
            const int untilFrame = mDebugServer->captureTicksUntilFrame(mSimulationClock.tick());
            if(untilFrame) maxSteps = untilFrame;
        }
#endif
        int steps = 0;
        int rendered = 0;
        while(open && !_window->wantsToClose && !(mInBackground && mSimulationClock.offlineCapture()) &&
              steps < maxSteps && mSimulationClock.beginTick()){
#ifdef DEBUGGING_TOOLS
            BaseSingleton::getDebugDrawer()->resetDraw();
#endif
            //Scene-safe callbacks and worker completions are simulation work, not repaint work.
            //Catch-up ticks share one render, so the previous tick's node changes have not reached
            //Ogre's derived transforms and bounds yet. Bring them up to date, otherwise ray queries
            //see wherever things were at the last render.
            if(mSceneGraphStale){
                _sceneManager->updateSceneGraph();
                mSceneGraphStale = false;
            }
            mScriptingStateManager->updateBaseState();
            mScriptPluginManager->updateSceneSafe();
            if(mScriptWorkerManager) mScriptWorkerManager->update();
            PhysicsBodyDestructor::update();

            //Consume the preceding tick's commands and publish physics before gameplay reads it.
            //No physics job survives a completed tick, including a script-requested pause.
            if(mThreadManager){
                mThreadManager->schedulePhysicsStep(mSimulationClock.deltaSeconds());
                mThreadManager->waitForPhysicsStep();
            }
            mPhysicsManager->update();
            mEntityManager->update();
            mScriptingStateManager->update();
            mScriptPluginManager->update();
            mScriptManager->processEvents();
            mAnimationManager->update();
            mInputManager->update(fixedDt);
            mGuiManager->update(fixedDt);
            mAudioManager->simulationTick(mSimulationClock.rate());

            mSimulationClock.finishTick();
            mSceneGraphStale = true;
#ifdef DEBUG_SERVER
            if(mDebugServer) mDebugServer->simulationTick();
#endif
            ++steps;
        }

        mAudioManager->setSimulationPaused(mSimulationClock.mode() != SimulationClock::Mode::RUNNING);
        if(open && !_window->wantsToClose && !mInBackground){
            mGuiManager->flushDirtyLabels();
#ifdef DEBUG_SERVER
            if(mDebugServer) mDebugServer->prepareVideoFrame(mSimulationClock.tick());
#endif
            //Repeated inspection renders contribute no simulation time. Catch-up ticks share
            //one render so vsync cannot limit the fixed update rate to the display refresh rate.
            _root->renderOneFrame(static_cast<Ogre::Real>(steps * mSimulationClock.deltaSeconds()));
            ++rendered;
            mSceneGraphStale = false;
        }

        //Ogre receives simulation deltas, so its synthetic frame statistics are not wall FPS.
        if(wallSeconds > 0 && mPreviousRenderCount > 0){
            const double frameSeconds = wallSeconds / mPreviousRenderCount;
            mAverageFrameSeconds = mAverageFrameSeconds == 0 ? frameSeconds : 0.9 * mAverageFrameSeconds + 0.1 * frameSeconds;
            BaseSingleton::mPerformanceStats.frameTime = static_cast<float>(frameSeconds * 1000);
            BaseSingleton::mPerformanceStats.fps = static_cast<float>(1.0 / frameSeconds);
            BaseSingleton::mPerformanceStats.avgFPS = static_cast<float>(1.0 / mAverageFrameSeconds);
        }
        mPreviousRenderCount = rendered;
#ifdef SCRIPT_PROFILER
        ScriptProfiler::notifyFrameBoundary();
#endif
#ifdef DEBUG_SERVER
        if(mDebugServer) mDebugServer->pumpMainThread();
#endif
        if(mSimulationClock.mode() == SimulationClock::Mode::PAUSED){
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }

    bool Base::isOpen(){
        //return _window->isOpen();
        return open;
    }

    int Base::getExitCode() const{
        return mExitCode;
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

        setup.setupCompositor(root, sceneManager, camera, _window->getRenderTexture());
        _sceneManager = sceneManager;
        //_sceneManager = std::shared_ptr<Ogre::SceneManager>(sceneManager);

        mGuiManager->setup(root, sceneManager);
        mGuiInputProcessor->initialise(mGuiManager.get());
    }

    void Base::shutdown(){
        #ifdef DEBUG_SERVER
            //Stop the debug server before any engine state it may be reading is torn down.
            if(mDebugServer){
                mDebugServer->shutdown();
                mDebugServer.reset();
            }
        #endif

        SystemEventEngineClose closeEvent;
        EventDispatcher::transmitEvent(EventType::System, closeEvent);


        if(mThreadManager) mThreadManager->notifyPhysicsManagerDestroyed();

        //The entity manager must go before the physics manager, as it holds references to physics objects.
        mEntityManager.reset();
        BaseSingleton::mEntityManager.reset();
        mPhysicsManager.reset();
        BaseSingleton::mPhysicsManager.reset();
        #ifdef DEBUGGING_TOOLS
            //This owns Ogre scene nodes, so it has to go before Ogre is torn down.
            //Physics destruction calls into it, so it has to go after the physics manager.
            mMeshVisualiser.reset();
            BaseSingleton::mMeshVisualiser.reset();
        #endif
        mScriptingStateManager->shutdown();
        //Script plugin end callbacks may still use namespaces supplied by native
        //plugins, so they run before those plugins detach from live services.
        mScriptPluginManager->shutdown();
        PluginManager::earlyShutdown();
        mGuiManager->shutdown();
        PhysicsCollisionDataManager::shutdown();
        if(mScriptWorkerManager) mScriptWorkerManager->shutdown();
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

        //The late phase remains after the vm has closed for plugins whose final
        //cleanup relies on every script handle already being gone.
        PluginManager::shutdown();
        delete _root;
        _window->close();
        PluginManager::unload();
        open = false;
    }
}
