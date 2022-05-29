#pragma once

#include <SDL_syswm.h>
#include <string>

#include "OgreWindow.h"

namespace AV{

    /**
     Get the window handle of an SDL cocoa window.

     @return
     A variable representing a pointer to the window. Pass that into ogre.
     */
    extern unsigned long WindowContentViewHandle(const SDL_SysWMinfo &info);

    /**
     Setup an SDL2 window with the correct ogre view.
     */
    void AssignViewToSDLWindow(const SDL_SysWMinfo &info, Ogre::Window* win);
}
