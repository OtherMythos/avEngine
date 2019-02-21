#pragma once

#include "OgreSetup.h"
#include "System/SystemSetup/SystemSettings.h"

#include "Ogre.h"
#include <OgreHlmsPbs.h>
#include <OgreHlmsUnlit.h>
#include "Logger/Log.h"
#include <Compositor/OgreCompositorManager2.h>


namespace AV{
    class WindowsOgreSetup : public OgreSetup{
    public:
		WindowsOgreSetup() {}

        Ogre::Root* setupRoot(){
            Ogre::Root *root = new Ogre::Root();

			root->loadPlugin("RenderSystem_Direct3D11_d");
            root->loadPlugin("RenderSystem_GL3Plus_d");
            root->setRenderSystem(root->getAvailableRenderers()[0]);
            root->getRenderSystem()->setConfigOption( "sRGB Gamma Conversion", "Yes" );
            root->initialise(false);

            return root;
        }

        void setupOgreWindow(Window *window){
            SDL2Window *sdlWindow = (SDL2Window*)window;

            Ogre::NameValuePairList params;

            params["externalWindowHandle"] = sdlWindow->getHandle();

			//TODO set the values for the window size based on data provided in the setup file.
            Ogre::RenderWindow *renderWindow = Ogre::Root::getSingleton().createRenderWindow("Ogre Window", 500, 400, false, &params);
            renderWindow->setVisible(true);

            sdlWindow->injectOgreWindow(renderWindow);
        }

        void setupHLMS(Ogre::Root *root){
            Ogre::RenderSystem *renderSystem = Ogre::Root::getSingletonPtr()->getRenderSystem();

            Ogre::ArchiveVec library;
            const std::string &rPath = SystemSettings::getMasterPath();

			if(renderSystem->getName() == "OpenGL 3+ Rendering Subsystem"){
				library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Common/GLSL", "FileSystem", true));
			}else{
				library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Common/HLSL", "FileSystem", true));
			}

            library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Unlit/Any", "FileSystem", true ));
            library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Pbs/Any", "FileSystem", true ));

            Ogre::Archive *archivePbs;
            Ogre::Archive *archiveUnlit;

			if(renderSystem->getName() == "OpenGL 3+ Rendering Subsystem"){
				archivePbs = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Pbs/GLSL", "FileSystem", true);
				archiveUnlit = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Unlit/GLSL", "FileSystem", true);
			}else{
				archivePbs = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Pbs/HLSL", "FileSystem", true);
				archiveUnlit = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Unlit/HLSL", "FileSystem", true);
			}
            Ogre::HlmsPbs *hlmsPbs = OGRE_NEW Ogre::HlmsPbs( archivePbs, &library );
            Ogre::HlmsUnlit *hlmsUnlit = OGRE_NEW Ogre::HlmsUnlit( archiveUnlit, &library );

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
