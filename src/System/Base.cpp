#include "Base.h"

#include "Logger/Log.h"
#include "Window/SDL2Window/SDL2Window.h"
#include "System/SystemSetup/SystemSetup.h"

#ifdef __APPLE__
    #include "OgreSetup/MacOSOgreSetup.h"
#endif


namespace AV {
    Base::Base(){
        SystemSetup::setup();
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
        _window->open();
        
        _setupOgre();
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
        #endif
        
        Ogre::Root *root = setup.setupRoot();
        _root = std::shared_ptr<Ogre::Root>(root);

        setup.setupOgreWindow((Window*)_window.get());
        setup.setupHLMS(root);
        
        Ogre::SceneManager *sceneManager;
        setup.setupScene(root, &sceneManager, &camera);
        setup.setupCompositor(root, sceneManager, camera, _window->getRenderWindow());
        _sceneManager = std::shared_ptr<Ogre::SceneManager>(sceneManager);
    }
    
    void Base::shutdown(){
        _window->close();
        _root->shutdown();
    }
}
