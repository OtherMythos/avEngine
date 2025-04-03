#pragma once

#include "OgreSetup.h"
#include "System/SystemSetup/SystemSettings.h"

#include "Ogre.h"
//#include <OgreHlmsPbs.h>
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
            Ogre::Root *root = _setupBasicOgreRoot();

            Ogre::String targetRenderSystem;
            auto system = SystemSettings::getCurrentRenderSystem();
            switch(system){
                case SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL:
                    targetRenderSystem = "RenderSystem_GL3Plus";
                    break;
                case SystemSettings::RenderSystemTypes::RENDER_SYSTEM_METAL:
                default:
                    targetRenderSystem = "RenderSystem_Metal";
                    break;
            }

            root->loadPlugin(targetRenderSystem, false, 0);
            root->loadPlugin("Plugin_ParticleFX", false, 0);

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

            //Ogre::RenderWindow *renderWindow = Ogre::Root::getSingleton().createRenderWindow("Ogre Window", 500, 400, false, &params);
            Ogre::uint32 width = SystemSettings::getDefaultWindowWidth();
            Ogre::uint32 height = SystemSettings::getDefaultWindowHeight();
            Ogre::Window *renderWindow = Ogre::Root::getSingleton().createRenderWindow("Ogre Window", width, height, false, &params);
            //renderWindow->setVisible(true);

            sdlWindow->injectOgreWindow(renderWindow);
        }

    };
}
