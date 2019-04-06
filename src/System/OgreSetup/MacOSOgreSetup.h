#pragma once

#include "OgreSetup.h"
#include "System/SystemSetup/SystemSettings.h"

#include "Ogre.h"
#include <OgreHlmsPbs.h>
#include <OgreHlmsUnlit.h>
#include "Logger/Log.h"
#include <Compositor/OgreCompositorManager2.h>


//#include <OgreMetalPlugin.h>
//#include <OgreGL3PlusPlugin.h>

namespace AV {
    /**
     An implementation of the ogre setup for MacOS.
     */
    class MacOSOgreSetup : public OgreSetup{
    public:
        MacOSOgreSetup() {}

        Ogre::Root* setupRoot(){
            Ogre::Root *root = new Ogre::Root();

            Ogre::String targetRenderSystem;
            auto system = SystemSettings::getCurrentRenderSystem();
            switch(system){
                case SystemSettings::RenderSystemTypes::RENDER_SYSTEM_METAL:
                    targetRenderSystem = "RenderSystem_Metal";
                    break;
                case SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL:
                    targetRenderSystem = "RenderSystem_GL3Plus";
                    break;
                default:
                    targetRenderSystem = "RenderSystem_Metal";
                    break;
            }
            root->loadPlugin(targetRenderSystem);

            //root->installPlugin(new Ogre::MetalPlugin());
            //root->installPlugin(new Ogre::GL3PlusPlugin());
            root->setRenderSystem(root->getAvailableRenderers()[0]);
            root->getRenderSystem()->setConfigOption( "sRGB Gamma Conversion", "Yes" );
            root->initialise(false);

            return root;
        }

        void setupOgreWindow(Window *window){
            SDL2Window *sdlWindow = (SDL2Window*)window;

            Ogre::NameValuePairList params;

            params.insert( std::make_pair("macAPI", "cocoa") );
            params.insert( std::make_pair("macAPICocoaUseNSView", "true") );
            params["parentWindowHandle"] = sdlWindow->getHandle();

            Ogre::RenderWindow *renderWindow = Ogre::Root::getSingleton().createRenderWindow("Ogre Window", 500, 400, false, &params);
            renderWindow->setVisible(true);

            sdlWindow->injectOgreWindow(renderWindow);
        }

        void setupHLMS(Ogre::Root *root){
            Ogre::RenderSystem *renderSystem = Ogre::Root::getSingletonPtr()->getRenderSystem();

            Ogre::ArchiveVec library;
            const std::string &rPath = SystemSettings::getMasterPath();

            if(renderSystem->getName() == "Metal Rendering Subsystem"){
                library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Common/Metal", "FileSystem", true ));
            }else if(renderSystem->getName() == "OpenGL 3+ Rendering Subsystem"){
                library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Common/GLSL", "FileSystem", true ));
            }

            library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Unlit/Any", "FileSystem", true ));
            library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Pbs/Any", "FileSystem", true ));
            //library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Common/Any", "FileSystem", true ));

            Ogre::Archive *archivePbs;
            Ogre::Archive *archiveUnlit;

            if(renderSystem->getName() == "OpenGL 3+ Rendering Subsystem"){
                archivePbs = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Pbs/GLSL", "FileSystem", true );
                archiveUnlit = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Unlit/GLSL", "FileSystem", true );
            }else{
                //If not opengl assume the render system is metal.
                archivePbs = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Pbs/Metal", "FileSystem", true );
                archiveUnlit = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Unlit/Metal", "FileSystem", true );
            }
            Ogre::HlmsPbs *hlmsPbs = new Ogre::HlmsPbs( archivePbs, &library );
            Ogre::HlmsUnlit *hlmsUnlit = new Ogre::HlmsUnlit( archiveUnlit, &library );

            root->getHlmsManager()->registerHlms(hlmsPbs);
            root->getHlmsManager()->registerHlms(hlmsUnlit);
        }

        void setupCompositor(Ogre::Root *root, Ogre::SceneManager* sceneManager, Ogre::Camera *camera, Ogre::RenderWindow *window){
            Ogre::CompositorManager2 *compositorManager = root->getCompositorManager2();

            const Ogre::String workspaceName("test Workspace");
            if(!compositorManager->hasWorkspaceDefinition(workspaceName)){
                compositorManager->createBasicWorkspaceDef(workspaceName, SystemSettings::getCompositorColourValue());
            }

            compositorManager->addWorkspace(sceneManager, window, camera, workspaceName, true);
        }
        
    };
}
