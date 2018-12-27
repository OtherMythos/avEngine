#include "Base.h"

#include "Logger/Log.h"
#include "Window/SDL2Window/SDL2Window.h"
#include "System/SystemSetup/SystemSetup.h"
#include "Scripting/ScriptManager.h"

#ifdef __APPLE__
    #include "OgreSetup/MacOSOgreSetup.h"
#elif __linux__
    #include "OgreSetup/LinuxOgreSetup.h"
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
        shutdown();
    }

    void Base::_initialise(){
        SystemSetup::setup();

        ScriptManager::initialise();
        _window->open();

        _setupOgre();

        ScriptManager::injectPointers(camera, _sceneManager);
        ScriptManager::runScript(SystemSettings::getResourcePath() + "/scripts/first.nut");
    }

    void Base::update(){
        Ogre::WindowEventUtilities::messagePump();
        _window->update();

        _root->renderOneFrame();
    }

    bool Base::isOpen(){
        return _window->isOpen();
    }

    void Base::_setupOgre(){
        #ifdef __APPLE__
        MacOSOgreSetup setup;
        #elif __linux__
        LinuxOgreSetup setup;
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
        _window->close();
        _root->shutdown();
    }
}
