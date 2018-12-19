#include "Base.h"

#include "Logger/Log.h"
#include "Window/SDL2Window/SDL2Window.h"
//#include "System/ResourcePathContainer.h"

#include "Ogre.h"


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
        _window->open();
        
        _setupOgre();
    }
    
    void Base::update(){
        _window->update();
    }
    
    bool Base::isOpen(){
        return _window->isOpen();
    }
    
    void Base::_setupOgre(){
        _setupOgreRoot();
        
        
    }
    
    void Base::_setupOgreRoot(){
        _root = std::make_shared<Ogre::Root>();
        
        _root->loadPlugin("RenderSystem_Metal");
        _root->setRenderSystem(_root->getAvailableRenderers()[0]);
        _root->getRenderSystem()->setConfigOption( "sRGB Gamma Conversion", "Yes" );
        _root->initialise(false);
    }
    
    void Base::_registerHLMS(){
//        Ogre::ArchiveVec library;
//        const std::string &rPath = ResourcePathContainer::getResourcePath();
//        library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Common/GLSL", "FileSystem", true ));
//        library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Unlit/Any", "FileSystem", true ));
//        
//        library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Pbs/Any", "FileSystem", true ));
//        library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Common/Any", "FileSystem", true ));
//        
//        Ogre::Archive *archivePbs;
//        Ogre::Archive *archiveUnlit;
//        
//        Ogre::RenderSystem *renderSystem = Ogre::Root::getSingletonPtr()->getRenderSystem();
//        if(renderSystem->getName() == "Metal Rendering Subsystem"){
//            archivePbs = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Pbs/Metal", "FileSystem", true );
//        }else{
//            archivePbs = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Pbs/GLSL", "FileSystem", true );
//            archiveUnlit = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Unlit/GLSL", "FileSystem", true );
//        }
//        Ogre::HlmsPbs *hlmsPbs = OGRE_NEW Ogre::HlmsPbs( archivePbs, &library );
//        Ogre::HlmsUnlit *hlmsUnlit = OGRE_NEW Ogre::HlmsUnlit( archiveUnlit, &library );
//        
//        root->getHlmsManager()->registerHlms(hlmsPbs);
//        root->getHlmsManager()->registerHlms(hlmsUnlit);
    }
    
    void Base::shutdown(){
        _root->shutdown();
        _window->close();
    }
}
