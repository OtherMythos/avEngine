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

    };
}
