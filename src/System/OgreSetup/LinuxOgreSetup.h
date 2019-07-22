#pragma once

#include "OgreSetup.h"
#include "System/SystemSetup/SystemSettings.h"

#include "Ogre.h"
#include <OgreHlmsPbs.h>
#include <OgreHlmsUnlit.h>
#include "Logger/Log.h"
#include <Compositor/OgreCompositorManager2.h>

#include "World/Slot/Chunk/Terrain/terra/Hlms/OgreHlmsTerra.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "World/Slot/Chunk/Terrain/terra/TerraShadowMapper.h"

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

            Ogre::RenderWindow *renderWindow = Ogre::Root::getSingleton().createRenderWindow("Ogre Window", 500, 400, false, &params);
            renderWindow->setVisible(true);

            sdlWindow->injectOgreWindow(renderWindow);
        }

        void setupHLMS(Ogre::Root *root){
            /*Ogre::RenderSystem *renderSystem = Ogre::Root::getSingletonPtr()->getRenderSystem();

            Ogre::ArchiveVec library;
            const std::string &rPath = SystemSettings::getMasterPath();

            library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Common/GLSL", "FileSystem", true ));

            library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Unlit/Any", "FileSystem", true ));
            library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Pbs/Any", "FileSystem", true ));

            Ogre::Archive *archivePbs;
            Ogre::Archive *archiveUnlit;

            archivePbs = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Pbs/GLSL", "FileSystem", true );
            archiveUnlit = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Unlit/GLSL", "FileSystem", true );
            Ogre::HlmsPbs *hlmsPbs = new Ogre::HlmsPbs( archivePbs, &library );
            Ogre::HlmsUnlit *hlmsUnlit = new Ogre::HlmsUnlit( archiveUnlit, &library );

            Ogre::Archive *archiveTerra = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Terra/GLSL", "FileSystem", true );

            root->getHlmsManager()->registerHlms(hlmsPbs);
            root->getHlmsManager()->registerHlms(hlmsUnlit);


            Ogre::HlmsTerra *hlmsTerra = new Ogre::HlmsTerra(archiveTerra, &library);
            root->getHlmsManager()->registerHlms(hlmsTerra);

            Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(false);*/



            Ogre::String dataFolder = SystemSettings::getMasterPath() + "/";
                    Ogre::String shaderSyntax = "GLSL";
                    #if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
                            Ogre::String rootHlmsFolder = Ogre::macBundlePath() + '/' +
                                                      cf.getSetting( "DoNotUseAsResource", "Hlms", "" );
                    #else
                            Ogre::String rootHlmsFolder = dataFolder;
                    #endif

                            if( rootHlmsFolder.empty() )
                                rootHlmsFolder = "./";
                            else if( *(rootHlmsFolder.end() - 1) != '/' )
                                rootHlmsFolder += "/";

                            //At this point rootHlmsFolder should be a valid path to the Hlms data folder

                            Ogre::HlmsUnlit *hlmsUnlit = 0;
                            Ogre::HlmsPbs *hlmsPbs = 0;

                            //For retrieval of the paths to the different folders needed
                            Ogre::String mainFolderPath;
                            Ogre::StringVector libraryFoldersPaths;
                            Ogre::StringVector::const_iterator libraryFolderPathIt;
                            Ogre::StringVector::const_iterator libraryFolderPathEn;

                            Ogre::ArchiveManager &archiveManager = Ogre::ArchiveManager::getSingleton();

                            {
                                //Create & Register HlmsUnlit
                                //Get the path to all the subdirectories used by HlmsUnlit
                                Ogre::HlmsUnlit::getDefaultPaths( mainFolderPath, libraryFoldersPaths );
                                Ogre::Archive *archiveUnlit = archiveManager.load( rootHlmsFolder + mainFolderPath,
                                                                                   "FileSystem", true );
                                Ogre::ArchiveVec archiveUnlitLibraryFolders;
                                libraryFolderPathIt = libraryFoldersPaths.begin();
                                libraryFolderPathEn = libraryFoldersPaths.end();
                                while( libraryFolderPathIt != libraryFolderPathEn )
                                {
                                    Ogre::Archive *archiveLibrary =
                                            archiveManager.load( rootHlmsFolder + *libraryFolderPathIt, "FileSystem", true );
                                    archiveUnlitLibraryFolders.push_back( archiveLibrary );
                                    ++libraryFolderPathIt;
                                }

                                //Create and register the unlit Hlms
                                hlmsUnlit = OGRE_NEW Ogre::HlmsUnlit( archiveUnlit, &archiveUnlitLibraryFolders );
                                Ogre::Root::getSingleton().getHlmsManager()->registerHlms( hlmsUnlit );
                            }

                            {
                                //Create & Register HlmsPbs
                                //Do the same for HlmsPbs:
                                Ogre::HlmsPbs::getDefaultPaths( mainFolderPath, libraryFoldersPaths );
                                Ogre::Archive *archivePbs = archiveManager.load( rootHlmsFolder + mainFolderPath,
                                                                                 "FileSystem", true );

                                //Get the library archive(s)
                                Ogre::ArchiveVec archivePbsLibraryFolders;
                                libraryFolderPathIt = libraryFoldersPaths.begin();
                                libraryFolderPathEn = libraryFoldersPaths.end();
                                while( libraryFolderPathIt != libraryFolderPathEn )
                                {
                                    Ogre::Archive *archiveLibrary =
                                            archiveManager.load( rootHlmsFolder + *libraryFolderPathIt, "FileSystem", true );
                                    archivePbsLibraryFolders.push_back( archiveLibrary );
                                    ++libraryFolderPathIt;
                                }

                                //Create and register
                                hlmsPbs = OGRE_NEW Ogre::HlmsPbs( archivePbs, &archivePbsLibraryFolders );
                                Ogre::Root::getSingleton().getHlmsManager()->registerHlms( hlmsPbs );
                            }


                            Ogre::RenderSystem *renderSystem = root->getRenderSystem();
                            if( renderSystem->getName() == "Direct3D11 Rendering Subsystem" )
                            {
                                //Set lower limits 512kb instead of the default 4MB per Hlms in D3D 11.0
                                //and below to avoid saturating AMD's discard limit (8MB) or
                                //saturate the PCIE bus in some low end machines.
                                bool supportsNoOverwriteOnTextureBuffers;
                                renderSystem->getCustomAttribute( "MapNoOverwriteOnDynamicBufferSRV",
                                                                  &supportsNoOverwriteOnTextureBuffers );

                                if( !supportsNoOverwriteOnTextureBuffers )
                                {
                                    hlmsPbs->setTextureBufferDefaultSize( 512 * 1024 );
                                    hlmsUnlit->setTextureBufferDefaultSize( 512 * 1024 );
                                }
                            }








                            //Ogre::String dataFolder = bundlePath + "/";
                                Ogre::String dFolder = "/home/edward/Documents/avDeps/ogre/Samples/Media/";

                                Ogre::String sSyntax = "GLSL";

                                std::vector<Ogre::String> cont;
                                cont.push_back(dFolder + "Hlms/Common/" + sSyntax);
                                cont.push_back(dFolder + "Hlms/Common/Any");
                                cont.push_back(dFolder + "Hlms/Pbs/Any");
                                cont.push_back(dFolder + "Hlms/Pbs/" + sSyntax);
                                Ogre::ArchiveVec library;

                                for(Ogre::String s : cont){
                                    Ogre::Archive *a = Ogre::ArchiveManager::getSingletonPtr()->load(s,"FileSystem", true );
                                    library.push_back(a);
                                    std::cout << s << '\n';
                                }

                                    Ogre::String s = dFolder + "Hlms/Terra/" + sSyntax;
                                    std::cout << s << '\n';
                                    Ogre::Archive *archiveTerra = Ogre::ArchiveManager::getSingletonPtr()->load(s, "FileSystem", true );
                                    Ogre::HlmsTerra *hlmsTerra = OGRE_NEW Ogre::HlmsTerra( archiveTerra, &library );
                                    Ogre::HlmsManager *hlmsManager = root->getHlmsManager();
                                    hlmsManager->registerHlms( hlmsTerra );


Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(false);

        }

        void setupCompositor(Ogre::Root *root, Ogre::SceneManager* sceneManager, Ogre::Camera *camera, Ogre::RenderWindow *window){
            /*Ogre::CompositorManager2 *compositorManager = root->getCompositorManager2();

            const Ogre::String workspaceName("test Workspace");
            if(!compositorManager->hasWorkspaceDefinition(workspaceName)){
                compositorManager->createBasicWorkspaceDef(workspaceName, SystemSettings::getCompositorColourValue());
            }

            compositorManager->addWorkspace(sceneManager, window, camera, workspaceName, true);*/

            using namespace Ogre;

//RenderWindow *renderWindow = getRenderWindow();
CompositorManager2 *compositorManager = root->getCompositorManager2();

//CompositorWorkspace *oldWorkspace = mGraphicsSystem->getCompositorWorkspace();
CompositorWorkspace *oldWorkspace = 0;
if( oldWorkspace )
{
    TexturePtr terraShadowTex = oldWorkspace->getExternalRenderTargets()[1].textures.back();
    if( terraShadowTex->getFormat() == PF_NULL )
    {
        ResourcePtr resourcePtr( terraShadowTex );
        TextureManager::getSingleton().remove( resourcePtr );
    }
    compositorManager->removeWorkspace( oldWorkspace );
}

CompositorChannelVec externalChannels( 2 );
//Render window
externalChannels[0].target = window;

//Terra's Shadow texture
ResourceLayoutMap initialLayouts;
ResourceAccessMap initialUavAccess;

{
    //The texture is not available. Create a dummy dud using PF_NULL.
    TexturePtr nullTex = TextureManager::getSingleton().createManual(
                "DummyNull", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                TEX_TYPE_2D, 1, 1, 0, PF_NULL );
    externalChannels[1].target = nullTex->getBuffer(0)->getRenderTarget();
    externalChannels[1].textures.push_back( nullTex );
}

//return compositorManager->addWorkspace( sceneManager, externalChannels, camera,
    compositorManager->addWorkspace( sceneManager, externalChannels, camera,
                                        "Tutorial_TerrainWorkspace", true, -1,
                                        (UavBufferPackedVec*)0, &initialLayouts,
                                        &initialUavAccess );

        }

    };
}
