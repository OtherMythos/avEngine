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
#include "System/Registry/ValueRegistry.h"

#include "World/Physics/PhysicsShapeManager.h"
#include "World/Physics/PhysicsBodyConstructor.h"
#include "World/Physics/PhysicsBodyDestructor.h"
#include "World/Slot/Chunk/TerrainManager.h"
#include "World/Support/OgreMeshManager.h"

#include "Input/InputManager.h"

#include "Threading/JobDispatcher.h"
#include "Threading/ThreadManager.h"
#include "BaseSingleton.h"

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

//Colibri stuff
#include "ColibriGui/ColibriManager.h"
#include "ColibriGui/Text/ColibriShaperManager.h"
#include "ColibriGui/Text/ColibriShaper.h"
#include "ColibriGui/Ogre/CompositorPassColibriGuiProvider.h"
#include "ColibriGui/ColibriWindow.h"
#include "ColibriGui/ColibriButton.h"
#include "ColibriGui/ColibriLabel.h"
#include "ColibriGui/Layouts/ColibriLayoutLine.h"
#include "hb.h"

namespace AV {
    Base::Base()
        : _window(std::make_shared<SDL2Window>()),
          mScriptingStateManager(std::make_shared<ScriptingStateManager>()),
          mSerialisationManager(std::make_shared<SerialisationManager>()),
          #ifdef DEBUGGING_TOOLS
            mImguiBase(std::make_shared<ImguiBase>()),
          #endif
          mThreadManager(std::make_shared<ThreadManager>()){

        auto rectMan = std::make_shared<Rect2dManager>();
        Window* win = (Window*)(_window.get());
        BaseSingleton::initialise(
            win,
            mScriptingStateManager,
            mSerialisationManager,
            std::make_shared<OgreMeshManager>(),
            rectMan,
            std::make_shared<DialogManager>(),
            std::make_shared<ValueRegistry>(),
            std::make_shared<TerrainManager>(),
            std::make_shared<InputManager>()
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
        auto inMan = BaseSingleton::getInputManager();
        inMan->setupDefaultActionSet();
        _window->open(inMan.get());

        _setupOgre();
        BaseSingleton::getOgreMeshManager()->setupSceneManager(_sceneManager);
        //BaseSingleton::getMovableTextureManager()->initialise(_sceneManager);
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

    static Colibri::ColibriManager* colibriManager = 0;
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

        colibriManager->update(10);

        _root->renderOneFrame();
    }

    bool Base::isOpen(){
        //return _window->isOpen();
        return open;
    }

    Colibri::Window *mainWindow = 0;
    Colibri::Button *button0 = 0;
    Colibri::Button *button1 = 0;

    void Base::_setupOgre(){
        #ifdef __APPLE__
        MacOSOgreSetup setup;
        #elif __linux__
        LinuxOgreSetup setup;
        #elif _WIN32
        WindowsOgreSetup setup;
        #endif

        Ogre::Root *root = setup.setupRoot();
        //_root = std::shared_ptr<Ogre::Root>(root);
        _root = root;


        static Colibri::LogListener colibriLogListener;
        static Colibri::ColibriListener colibriListener;
        colibriManager = new Colibri::ColibriManager( &colibriLogListener, &colibriListener );


        setup.setupOgreWindow((Window*)_window.get());
        Ogre::CompositorPassColibriGuiProvider *compoProvider =
                OGRE_NEW Ogre::CompositorPassColibriGuiProvider( colibriManager );
        Ogre::CompositorManager2 *compositorManager = root->getCompositorManager2();
        compositorManager->setCompositorPassProvider( compoProvider );

        setup.setupOgreResources(root);
        setup.setupHLMS(root);

        Ogre::SceneManager *sceneManager;
        setup.setupScene(root, &sceneManager, &camera);
        setup.setupCompositor(root, sceneManager, camera, _window->getRenderWindow());
        _sceneManager = sceneManager;
        //_sceneManager = std::shared_ptr<Ogre::SceneManager>(sceneManager);

        //Colibri stuff
        {


            struct ShaperSettings
                {
                    const char *locale;
                    const char *fullpath;
                    hb_script_t script;
                    Colibri::HorizReadingDir::HorizReadingDir horizReadingDir;
                    bool useKerning;
                    bool allowsVerticalLayout;
                    ShaperSettings( const char *_locale, const char *_fullpath, hb_script_t _script,
                                    bool _useKerning=false,
                                    Colibri::HorizReadingDir::HorizReadingDir _horizReadingDir=
                            Colibri::HorizReadingDir::LTR,
                                    bool _allowsVerticalLayout=false ) :
                        locale( _locale ),
                        fullpath( _fullpath ),
                        script( _script ),
                        horizReadingDir( _horizReadingDir ),
                        useKerning( _useKerning ),
                        allowsVerticalLayout( _allowsVerticalLayout )
                    {

                    }
                };

                ShaperSettings shaperSettings[3] =
                {
                    ShaperSettings( "en", "/home/edward/Documents/avDeps/colibrigui/bin/Data/Fonts/DejaVuSerif.ttf", HB_SCRIPT_LATIN, true ),
                    ShaperSettings( "ar", "/home/edward/Documents/avDeps/colibrigui/bin/Data/Fonts/amiri-0.104/amiri-regular.ttf", HB_SCRIPT_ARABIC, false,
                    Colibri::HorizReadingDir::RTL ),
                    ShaperSettings( "ch", "/home/edward/Documents/avDeps/colibrigui/bin/Data/Fonts/fireflysung-1.3.0/fireflysung.ttf", HB_SCRIPT_HAN, false,
                    Colibri::HorizReadingDir::LTR, true )
                };

                Colibri::ShaperManager *shaperManager = colibriManager->getShaperManager();

                for( size_t i=0; i<sizeof( shaperSettings ) / sizeof( shaperSettings[0] ); ++i )
                {
                    Colibri::Shaper *shaper;
                    shaper = shaperManager->addShaper( shaperSettings[i].script, shaperSettings[i].fullpath,
                                                       shaperSettings[i].locale );
                    if( shaperSettings[i].useKerning )
                        shaper->addFeatures( Colibri::Shaper::KerningOn );
                }

                size_t defaultFont = 0; //"en"
                shaperManager->setDefaultShaper( defaultFont + 1u,
                                                 shaperSettings[defaultFont].horizReadingDir,
                                                 shaperSettings[defaultFont].allowsVerticalLayout );

                if( defaultFont == 1 )
                    colibriManager->setSwapRTLControls( true );

        }

        colibriManager->setOgre( root,
                         root->getRenderSystem()->getVaoManager(),
                         sceneManager );
        colibriManager->loadSkins("/home/edward/Documents/avDeps/colibrigui/bin/Data/Materials/ColibriGui/Skins/DarkGloss/Skins.colibri.json");

        mainWindow = colibriManager->createWindow( 0 );
        mainWindow->setTransform( Ogre::Vector2( 0, 0 ), Ogre::Vector2( 450, 0 ) );

        colibriManager->setCanvasSize( Ogre::Vector2( 1920.0f, 1080.0f ), Ogre::Vector2( _window->getWidth(), _window->getHeight() ) );

        Colibri::LayoutLine *layout = new Colibri::LayoutLine( colibriManager );

        button0 = colibriManager->createWidget<Colibri::Button>( mainWindow );
        button0->m_minSize = Ogre::Vector2( 350, 64 );
        button0->getLabel()->setText( "This is a button" );
        button0->sizeToFit();
        layout->addCell( button0 );

        button1 = colibriManager->createWidget<Colibri::Button>( mainWindow );
        button1->m_minSize = Ogre::Vector2( 350, 64 );
        button1->getLabel()->setText( "This is another button" );
        button1->sizeToFit();
        layout->addCell( button1 );

        layout->setAdjustableWindow( mainWindow );
        layout->m_hardMaxSize = colibriManager->getCanvasSize();

        Colibri::LayoutLine *layoutW = new Colibri::LayoutLine( colibriManager );
        layoutW->setCellSize( colibriManager->getCanvasSize() );
        layoutW->addCell( &Colibri::LayoutSpacer::c_DefaultBlankSpacer );
        layoutW->addCell( layout );
        layoutW->layout();
    }

    void Base::shutdown(){
        SystemEventEngineClose closeEvent;
        EventDispatcher::transmitEvent(EventType::System, closeEvent);

        #ifdef DEBUGGING_TOOLS
            mImguiBase.reset();
        #endif

        BaseSingleton::getDialogManager()->shutdown();

        WorldSingleton::destroyWorld();
        mScriptingStateManager->shutdown();
        ScriptManager::shutdown();
        JobDispatcher::shutdown();
        PhysicsBodyConstructor::shutdown();
        PhysicsBodyDestructor::shutdown();
        PhysicsShapeManager::shutdown();
        ProgrammaticMeshGenerator::shutdown();
        //_root->shutdown();
        _window->close();
        delete _root;
        open = false;
    }
}
