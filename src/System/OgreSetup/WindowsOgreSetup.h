#pragma once

#include "OgreSetup.h"
#include "System/SystemSetup/SystemSettings.h"

#include "Ogre.h"
#include <OgreHlmsPbs.h>
#include <OgreHlmsUnlit.h>
#include <OgreWindow.h>
#include "Logger/Log.h"
#include "RenderSystems/Direct3D11/OgreD3D11Device.h"
#include "System/OgreSetup/CustomHLMS/OgreHlmsPbsAVCustom.h"

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

            Ogre::uint32 width = SystemSettings::getDefaultWindowWidth();
            Ogre::uint32 height = SystemSettings::getDefaultWindowHeight();
            Ogre::Window *renderWindow = Ogre::Root::getSingleton().createRenderWindow("Ogre Window", width, height, false, &params);
            renderWindow->setVSync(true, 1);

            sdlWindow->injectOgreWindow(renderWindow);
        }

    };
}
