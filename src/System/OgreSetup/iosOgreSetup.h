#pragma once

#include "OgreSetup.h"
#include "System/SystemSetup/SystemSettings.h"
#include "System/OgreSetup/CustomHLMS/OgreHlmsPbsAVCustom.h"

#include "World/Slot/Chunk/Terrain/terra/Hlms/OgreHlmsTerra.h"

#include "Ogre.h"
#include <OgreHlmsUnlit.h>
#include "Logger/Log.h"
#include <Compositor/OgreCompositorManager2.h>
#include "ColibriGui/Ogre/OgreHlmsColibri.h"

#include "Window/SDL2Window/SDL2Window.h"

#include <OgreMetalPlugin.h>
#include <OgreParticleFXPlugin.h>

namespace AV {
    /**
     An implementation of the ogre setup for iOS.
     */
    class iosOgreSetup : public OgreSetup{
    public:
        iosOgreSetup() {}

        Ogre::Root* setupRoot(){
            Ogre::Root *root = _setupBasicOgreRoot();

            //TODO will want to destroy these I assume.
            root->installPlugin(new Ogre::MetalPlugin(), 0);
            root->installPlugin(new Ogre::ParticleFXPlugin(), 0);

            root->setRenderSystem(root->getAvailableRenderers()[0]);
            root->getRenderSystem()->setConfigOption( "sRGB Gamma Conversion", "Yes" );
            root->initialise(false);

            return root;
        }

        void setupOgreWindow(Window *window){
            SDL2Window *sdlWindow = (SDL2Window*)window;

            Ogre::NameValuePairList params;

            Ogre::Window *renderWindow = Ogre::Root::getSingleton().createRenderWindow("Ogre Window", 500, 500, false, &params);

            window->injectOgreWindow(renderWindow);

            sdlWindow->extraOgreSetup();
        }

        void setupHLMS(Ogre::Root *root){
            Ogre::RenderSystem *renderSystem = Ogre::Root::getSingletonPtr()->getRenderSystem();

            Ogre::ArchiveVec library;
            const std::string &rPath = SystemSettings::getMasterPath();

            Ogre::String mainFolderPath;
            Ogre::StringVector libraryFoldersPaths;
            Ogre::StringVector::const_iterator libraryFolderPathIt;
            Ogre::StringVector::const_iterator libraryFolderPathEn;

            Ogre::ArchiveManager &archiveManager = Ogre::ArchiveManager::getSingleton();

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
                hlmsPbs = OGRE_NEW HlmsPbsAVCustom( archivePbs, &archivePbsLibraryFolders );
                Root::getSingleton().getHlmsManager()->registerHlms( hlmsPbs );
            }

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
