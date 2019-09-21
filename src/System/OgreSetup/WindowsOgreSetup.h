#pragma once

#include "OgreSetup.h"
#include "System/SystemSetup/SystemSettings.h"

#include "Ogre.h"
#include <OgreHlmsPbs.h>
#include <OgreHlmsUnlit.h>
#include "Logger/Log.h"
#include "OgreD3D11Device.h"

#include "World/Slot/Chunk/Terrain/terra/Hlms/OgreHlmsTerra.h"


namespace AV{
    class WindowsOgreSetup : public OgreSetup{
    public:
        WindowsOgreSetup() {}

        Ogre::Root* setupRoot(){
            Ogre::Root *root = new Ogre::Root();

            Ogre::String targetRenderSystem;
            auto system = SystemSettings::getCurrentRenderSystem();
            switch (system) {
                case SystemSettings::RenderSystemTypes::RENDER_SYSTEM_D3D11:
                    targetRenderSystem = "RenderSystem_Direct3D11_d";
                    break;
                case SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL:
                    targetRenderSystem = "RenderSystem_GL3Plus_d";
                    break;
                default:
                    targetRenderSystem = "RenderSystem_Direct3D11_d";
                    break;
            }
            root->loadPlugin(targetRenderSystem);
            //root->loadPlugin("RenderSystem_GL3Plus_d");
            root->setRenderSystem(root->getAvailableRenderers()[0]);
            root->getRenderSystem()->setConfigOption( "sRGB Gamma Conversion", "Yes" );
            root->initialise(false);

            //Necessary for imgui, although that should only apply for debug builds.
            Ogre::D3D11Device::setExceptionsErrorLevel(Ogre::D3D11Device::D3D_NO_EXCEPTION);

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

            const Ogre::String renderSystemPrefix = renderSystem->getName() == "OpenGL 3+ Rendering Subsystem" ? "GLSL" : "HLSL";

            library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Common/" + renderSystemPrefix, "FileSystem", true));

            library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Unlit/Any", "FileSystem", true ));
            library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Pbs/Any", "FileSystem", true ));

            Ogre::Archive *archivePbs;
            Ogre::Archive *archiveUnlit;

            archivePbs = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Pbs/" + renderSystemPrefix, "FileSystem", true);
            archiveUnlit = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Unlit/" + renderSystemPrefix, "FileSystem", true);

            Ogre::HlmsPbs *hlmsPbs = OGRE_NEW Ogre::HlmsPbs( archivePbs, &library );
            Ogre::HlmsUnlit *hlmsUnlit = OGRE_NEW Ogre::HlmsUnlit( archiveUnlit, &library );

            root->getHlmsManager()->registerHlms(hlmsPbs);
            root->getHlmsManager()->registerHlms(hlmsUnlit);


            //----
            //Register the Terra HLMS

            {
                std::vector<Ogre::String> cont;
                cont.push_back(rPath + "Hlms/Common/" + renderSystemPrefix);
                cont.push_back(rPath + "Hlms/Common/Any");
                cont.push_back(rPath + "Hlms/Pbs/Any");
                cont.push_back(rPath + "Hlms/Pbs/" + renderSystemPrefix);
                Ogre::ArchiveVec l;

                for(Ogre::String s : cont){
                    Ogre::Archive *a = Ogre::ArchiveManager::getSingletonPtr()->load(s,"FileSystem", true );
                    l.push_back(a);
                }

                Ogre::Archive *archiveTerra = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "Hlms/Terra/" + renderSystemPrefix, "FileSystem", true );
                Ogre::HlmsTerra *hlmsTerra = OGRE_NEW Ogre::HlmsTerra( archiveTerra, &l );
                Ogre::HlmsManager *hlmsManager = root->getHlmsManager();
                hlmsManager->registerHlms( hlmsTerra );
            }

            Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(false);
        }

    };
}
