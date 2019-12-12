#pragma once

#include "OgreSetup.h"
#include "System/SystemSetup/SystemSettings.h"

#include "Ogre.h"
#include <OgreHlmsPbs.h>
#include <OgreHlmsUnlit.h>
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

            Ogre::RenderWindow *renderWindow = Ogre::Root::getSingleton().createRenderWindow("Ogre Window", 500, 400, false, &params);
            renderWindow->setVisible(true);
            renderWindow->setVSyncEnabled(true);

            sdlWindow->injectOgreWindow(renderWindow);
        }

        void setupHLMS(Ogre::Root *root){
            Ogre::RenderSystem *renderSystem = Ogre::Root::getSingletonPtr()->getRenderSystem();
            const std::string &rPath = SystemSettings::getMasterPath();

            {
                Ogre::ArchiveVec library;

                library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Common/GLSL", "FileSystem", true ));

                library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Unlit/Any", "FileSystem", true ));
                library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Pbs/Any", "FileSystem", true ));

                Ogre::Archive *archivePbs;
                Ogre::Archive *archiveUnlit;

                archivePbs = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Pbs/GLSL", "FileSystem", true );
                archiveUnlit = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Unlit/GLSL", "FileSystem", true );
                Ogre::HlmsPbs *hlmsPbs = new Ogre::HlmsPbs( archivePbs, &library );
                Ogre::HlmsUnlit *hlmsUnlit = new Ogre::HlmsUnlit( archiveUnlit, &library );

                root->getHlmsManager()->registerHlms(hlmsPbs);
                root->getHlmsManager()->registerHlms(hlmsUnlit);
            }

            //----
            //Register the Terra HLMS

            {
                std::vector<Ogre::String> cont;
                cont.push_back(rPath + "Hlms/Common/GLSL");
                cont.push_back(rPath + "Hlms/Common/Any");
                cont.push_back(rPath + "Hlms/Pbs/Any");
                cont.push_back(rPath + "Hlms/Pbs/GLSL");
                Ogre::ArchiveVec l;

                for(Ogre::String s : cont){
                    Ogre::Archive *a = Ogre::ArchiveManager::getSingletonPtr()->load(s,"FileSystem", true );
                    l.push_back(a);
                }

                Ogre::Archive *archiveTerra = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "Hlms/Terra/GLSL", "FileSystem", true );
                Ogre::HlmsTerra *hlmsTerra = OGRE_NEW Ogre::HlmsTerra( archiveTerra, &l );
                Ogre::HlmsManager *hlmsManager = root->getHlmsManager();
                hlmsManager->registerHlms( hlmsTerra );
            }

            Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(false);

        }

    };
}
