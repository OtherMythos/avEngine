#pragma once

#include "OgreSetup.h"
#include "System/SystemSetup/SystemSettings.h"

#include "Ogre.h"
#include <OgreHlmsPbs.h>
#include <OgreHlmsUnlit.h>
#include <OgreWindow.h>
#include "Logger/Log.h"

#include "World/Slot/Chunk/Terrain/terra/Hlms/OgreHlmsTerra.h"
#include "Compositor/OgreCompositorWorkspace.h"

#include "filesystem/path.h"

namespace AV{
    class LinuxOgreSetup : public OgreSetup{
    public:
        LinuxOgreSetup() {}

        Ogre::Root* setupRoot(){
            Ogre::Root *root = new Ogre::Root();

            //I've found that in Linux (specifically mint), you need to have an absolute path to the shared object.
            //For some reason you can just name it in arch.
            //I'd say that the buck probably stops with ogre on that one, and it's doing something different per distribution, but that's just a hypothesis. I haven't checked any source code.
            //However I might as well just do an absolute path on all distributions, it'll just make it more reliable at the end of the day.
            //You can just sym link to the libs if necessary, but they now need to be in the master directory.
            filesystem::path masterPath(SystemSettings::getMasterPath());
            filesystem::path finalPath = masterPath / filesystem::path("RenderSystem_GL3Plus.so");

            root->loadPlugin(finalPath.str());
            root->setRenderSystem(root->getAvailableRenderers()[0]);
            root->getRenderSystem()->setConfigOption( "sRGB Gamma Conversion", "Yes" );
            root->initialise(false);

            return root;
        }

        void setupOgreWindow(Window *window){
            SDL2Window *sdlWindow = (SDL2Window*)window;

            Ogre::NameValuePairList params;

            params["parentWindowHandle"] = sdlWindow->getHandle();

            Ogre::Window *renderWindow = Ogre::Root::getSingleton().createRenderWindow("Ogre Window", 500, 400, false, &params);
            //renderWindow->setVisible(true);
            renderWindow->setVSync(true, 60);

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
                HlmsUnlit *hlmsUnlit = 0;

                // Create & Register HlmsUnlit
                // Get the path to all the subdirectories used by HlmsUnlit
                HlmsUnlit::getDefaultPaths( mainFolderPath, libraryFoldersPaths );
                Archive *archiveUnlit =
                    archiveManager.load( rPath + mainFolderPath, "FileSystem", true );
                ArchiveVec archiveUnlitLibraryFolders;
                libraryFolderPathIt = libraryFoldersPaths.begin();
                libraryFolderPathEn = libraryFoldersPaths.end();
                while( libraryFolderPathIt != libraryFolderPathEn )
                {
                    Archive *archiveLibrary =
                        archiveManager.load( rPath + *libraryFolderPathIt, "FileSystem", true );
                    archiveUnlitLibraryFolders.push_back( archiveLibrary );
                    ++libraryFolderPathIt;
                }

                // Create and register the unlit Hlms
                hlmsUnlit = OGRE_NEW HlmsUnlit( archiveUnlit, &archiveUnlitLibraryFolders );
                Root::getSingleton().getHlmsManager()->registerHlms( hlmsUnlit );
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

                //Create and register the terra Hlms
                hlmsTerra = OGRE_NEW Ogre::HlmsTerra( archiveTerra, &archiveTerraLibraryFolders );
                hlmsManager->registerHlms( hlmsTerra );
            }

            Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(false);

        }

    };
}
