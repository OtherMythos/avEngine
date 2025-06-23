#pragma once

#include "OgreSetup.h"
#include "System/SystemSetup/SystemSettings.h"

#include "Ogre.h"
#include <OgreHlmsPbs.h>
#include <OgreHlmsUnlit.h>
#include <OgreWindow.h>
#include "Logger/Log.h"

#include "System/OgreSetup/CustomHLMS/OgreHlmsPbsAVCustom.h"
#include "ColibriGui/Ogre/OgreHlmsColibri.h"
#include "World/Slot/Chunk/Terrain/terra/Hlms/OgreHlmsTerra.h"
#include "Compositor/OgreCompositorWorkspace.h"

#include <SDL.h>
#include <SDL_syswm.h>

#include "filesystem/path.h"

namespace AV{
    class LinuxOgreSetup : public OgreSetup{
    public:
        LinuxOgreSetup() {}

        Ogre::Root* setupRoot(){
            Ogre::Root *root = _setupBasicOgreRoot();

            //I've found that in Linux (specifically mint), you need to have an absolute path to the shared object.
            //For some reason you can just name it in arch.
            //I'd say that the buck probably stops with ogre on that one, and it's doing something different per distribution, but that's just a hypothesis. I haven't checked any source code.
            //However I might as well just do an absolute path on all distributions, it'll just make it more reliable at the end of the day.
            //You can just sym link to the libs if necessary, but they now need to be in the master directory.
            filesystem::path masterPath(SystemSettings::getMasterPath());
            filesystem::path renderSystemPath = masterPath / filesystem::path("RenderSystem_GL3Plus.so");
            filesystem::path renderSystemVulkanPath = masterPath / filesystem::path("RenderSystem_Vulkan.so");
            filesystem::path particleFXPath = masterPath / filesystem::path("Plugin_ParticleFX.so");

            root->loadPlugin(renderSystemPath.str(), false, 0);
            root->loadPlugin(renderSystemVulkanPath.str(), true, 0);
            root->loadPlugin(particleFXPath.str(), false, 0);

            size_t arrayIdx = 0;
            SystemSettings::RenderSystemTypes renderSystem = SystemSettings::getCurrentRenderSystem();
            if(renderSystem == SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL){
                arrayIdx = 0;
            }
            else if(renderSystem == SystemSettings::RenderSystemTypes::RENDER_SYSTEM_VULKAN){
                arrayIdx = 1;
            }

            root->setRenderSystem(root->getAvailableRenderers()[arrayIdx]);
            root->initialise(false);

            return root;
        }

        void setupOgreWindow(Window *window){
            SDL2Window *sdlWindow = (SDL2Window*)window;

            Ogre::NameValuePairList params;

            params["parentWindowHandle"] = sdlWindow->getHandle();
            SDL_SysWMinfo wmInfo;
            params["SDL2x11"] = sdlWindow->getX11Handle(&wmInfo);
            params["gamma"] = "yes";

            Ogre::uint32 width = SystemSettings::getDefaultWindowWidth();
            Ogre::uint32 height = SystemSettings::getDefaultWindowHeight();
            Ogre::Window *renderWindow = Ogre::Root::getSingleton().createRenderWindow("Ogre Window", width, height, false, &params);
            //renderWindow->setVisible(true);
            if(!SystemSettings::getForceDisableVsync()){
                renderWindow->setVSync(true, 1);
            }

            sdlWindow->injectOgreWindow(renderWindow);
        }

    };
}
