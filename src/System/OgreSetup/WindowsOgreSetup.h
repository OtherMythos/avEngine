#pragma once

#include "OgreSetup.h"
#include "System/SystemSetup/SystemSettings.h"

#include "Ogre.h"
#include <OgreHlmsPbs.h>
#include <OgreHlmsUnlit.h>
#include <OgreWindow.h>
#include "Logger/Log.h"
#include "RenderSystems/Direct3D11/OgreD3D11Device.h"

#include "ColibriGui/Ogre/OgreHlmsColibri.h"
#include "World/Slot/Chunk/Terrain/terra/Hlms/OgreHlmsTerra.h"


namespace AV{
    class WindowsOgreSetup : public OgreSetup{
    public:
        WindowsOgreSetup() {}

        Ogre::Root* setupRoot(){
            Ogre::Root *root = _setupBasicOgreRoot();

            Ogre::String targetRenderSystem;
            auto system = SystemSettings::getCurrentRenderSystem();
            switch (system) {
                case SystemSettings::RenderSystemTypes::RENDER_SYSTEM_D3D11:
                    targetRenderSystem = "RenderSystem_Direct3D11";
                    break;
                case SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL:
                    targetRenderSystem = "RenderSystem_GL3Plus";
                    break;
                case SystemSettings::RenderSystemTypes::RENDER_SYSTEM_VULKAN:
                    targetRenderSystem = "RenderSystem_Vulkan";
                    break;
                default:
                    targetRenderSystem = "RenderSystem_Direct3D11";
                    break;
            }
            Ogre::String particleFxPlugin = "Plugin_ParticleFX";
            #if OGRE_DEBUG_MODE
            targetRenderSystem += "_d";
            particleFxPlugin += "_d";
            #endif
            root->loadPlugin(targetRenderSystem, false, 0);
            root->loadPlugin(particleFxPlugin, false, 0);
            root->setRenderSystem(root->getAvailableRenderers()[0]);
            root->initialise(false);

            return root;
        }

        void setupOgreWindow(Window *window){
            SDL2Window *sdlWindow = (SDL2Window*)window;

            Ogre::NameValuePairList params;

            params["externalWindowHandle"] = sdlWindow->getHandle();
            params["gamma"] = "yes";

            Ogre::Window *renderWindow = Ogre::Root::getSingleton().createRenderWindow("Ogre Window", 500, 400, false, &params);
            renderWindow->setVSync(true, 1);

            sdlWindow->injectOgreWindow(renderWindow);
        }

        void setupHLMS(Ogre::Root *root){
            Ogre::RenderSystem *renderSystem = Ogre::Root::getSingletonPtr()->getRenderSystem();
            const std::string &rPath = SystemSettings::getMasterPath();

            // For retrieval of the paths to the different folders needed
            Ogre::String mainFolderPath;
            Ogre::StringVector libraryFoldersPaths;
            Ogre::StringVector::const_iterator libraryFolderPathIt;
            Ogre::StringVector::const_iterator libraryFolderPathEn;

            Ogre::ArchiveManager &archiveManager = Ogre::ArchiveManager::getSingleton();

            {
                using namespace Ogre;
                //Create & Register HlmsColibri
                //Get the path to all the subdirectories used by HlmsColibri
                Ogre::HlmsColibri::getDefaultPaths( mainFolderPath, libraryFoldersPaths );
                Ogre::Archive *archiveUnlit = archiveManager.load( rPath + mainFolderPath,
                                                                   "FileSystem", true );
                Ogre::ArchiveVec archiveUnlitLibraryFolders;
                libraryFolderPathIt = libraryFoldersPaths.begin();
                libraryFolderPathEn = libraryFoldersPaths.end();
                while( libraryFolderPathIt != libraryFolderPathEn )
                {
                    Ogre::Archive *archiveLibrary =
                            archiveManager.load( rPath + *libraryFolderPathIt, "FileSystem", true );
                    archiveUnlitLibraryFolders.push_back( archiveLibrary );
                    ++libraryFolderPathIt;
                }
                _registerHlmsPieceLibraryToHlms("unlit", archiveUnlitLibraryFolders);

                //Create and register the unlit Hlms
                Ogre::HlmsColibri* hlmsColibri = OGRE_NEW Ogre::HlmsColibri( archiveUnlit, &archiveUnlitLibraryFolders );
                Ogre::Root::getSingleton().getHlmsManager()->registerHlms( hlmsColibri );
            }

            {
                using namespace Ogre;
                HlmsPbs *hlmsPbs = 0;

                // Create & Register HlmsPbs
                // Do the same for HlmsPbs:
                HlmsPbs::getDefaultPaths( mainFolderPath, libraryFoldersPaths );
                Archive *archivePbs = archiveManager.load( rPath + mainFolderPath, "FileSystem", true );

                // Get the library archive(s)
                ArchiveVec archivePbsLibraryFolders;
                libraryFolderPathIt = libraryFoldersPaths.begin();
                libraryFolderPathEn = libraryFoldersPaths.end();
                while( libraryFolderPathIt != libraryFolderPathEn )
                {
                    Archive *archiveLibrary =
                        archiveManager.load( rPath + *libraryFolderPathIt, "FileSystem", true );
                    archivePbsLibraryFolders.push_back( archiveLibrary );
                    ++libraryFolderPathIt;
                }
                _registerHlmsPieceLibraryToHlms("pbs", archivePbsLibraryFolders);

                // Create and register
                hlmsPbs = OGRE_NEW HlmsPbs( archivePbs, &archivePbsLibraryFolders );
                Root::getSingleton().getHlmsManager()->registerHlms( hlmsPbs );
            }

            //----
            //Register the Terra HLMS

            {
                Ogre::String mainFolderPath;
                Ogre::StringVector libraryFoldersPaths;
                Ogre::StringVector::const_iterator libraryFolderPathIt;
                Ogre::StringVector::const_iterator libraryFolderPathEn;

                Ogre::ArchiveManager &archiveManager = Ogre::ArchiveManager::getSingleton();

                Ogre::HlmsTerra *hlmsTerra = 0;
                Ogre::HlmsManager *hlmsManager = root->getHlmsManager();

                //Create & Register HlmsTerra
                //Get the path to all the subdirectories used by HlmsTerra
                Ogre::HlmsTerra::getDefaultPaths( mainFolderPath, libraryFoldersPaths );
                Ogre::Archive *archiveTerra = archiveManager.load( rPath + mainFolderPath,
                                                                   "FileSystem", true );
                Ogre::ArchiveVec archiveTerraLibraryFolders;
                libraryFolderPathIt = libraryFoldersPaths.begin();
                libraryFolderPathEn = libraryFoldersPaths.end();
                while( libraryFolderPathIt != libraryFolderPathEn )
                {
                    Ogre::Archive *archiveLibrary = archiveManager.load( rPath +
                                                                         *libraryFolderPathIt,
                                                                         "FileSystem", true );
                    archiveTerraLibraryFolders.push_back( archiveLibrary );
                    ++libraryFolderPathIt;
                }
                _registerHlmsPieceLibraryToHlms("terra", archiveTerraLibraryFolders);

                //Create and register the terra Hlms
                hlmsTerra = OGRE_NEW Ogre::HlmsTerra( archiveTerra, &archiveTerraLibraryFolders );
                hlmsManager->registerHlms( hlmsTerra );
            }

            Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(false);
        }

    };
}
