#pragma once

#include "OgreSetup.h"
#include "System/SystemSetup/SystemSettings.h"

#include "Ogre.h"
#include <OgreHlmsPbs.h>
#include <OgreHlmsUnlit.h>
#include <OgreWindow.h>
#include "Logger/Log.h"

#include "ColibriGui/Ogre/OgreHlmsColibri.h"
#include "World/Slot/Chunk/Terrain/terra/Hlms/OgreHlmsTerra.h"
#include "Compositor/OgreCompositorWorkspace.h"

#include <SDL.h>
#include <SDL_syswm.h>

#include <OgreVulkanPlugin.h>
#include <OgreParticleFXPlugin.h>

#include "filesystem/path.h"

#include "AndroidSetupHelper.h"

namespace AV{
    class AndroidOgreSetup : public OgreSetup{
    public:
        AndroidOgreSetup() {}

        Ogre::Root* setupRoot(){
            Ogre::Root *root = new Ogre::Root();

            root->installPlugin(new Ogre::VulkanPlugin(), 0);
            root->installPlugin(new Ogre::ParticleFXPlugin(), 0);

            root->setRenderSystem(root->getAvailableRenderers()[0]);
            root->initialise(false);

            //This has to be setup in time for any ogre file access.
            AndroidSetupHelper::setupOgreArchiveFactories();

            return root;
        }

        void setupOgreWindow(Window *window){
            SDL2Window *sdlWindow = (SDL2Window*)window;

            Ogre::NameValuePairList params;

            params.insert( std::make_pair(
                "ANativeWindow",
                sdlWindow->getHandle() ) );

            params["gamma"] = "yes";

            Ogre::Window *renderWindow = Ogre::Root::getSingleton().createRenderWindow("Ogre Window", 500, 400, false, &params);
            //renderWindow->setVisible(true);
            if(!SystemSettings::getForceDisableVsync()){
                renderWindow->setVSync(true, 1);
            }

            sdlWindow->injectOgreWindow(renderWindow);
        }
    };
}
