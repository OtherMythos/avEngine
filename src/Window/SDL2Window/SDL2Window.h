#pragma once

#include "../Window.h"

class SDL_Window;

namespace AV {
    class SDL2Window : public Window{
    public:
        SDL2Window();
        ~SDL2Window();
        
        /**
        updates the window.
         
         @remarks
         This includes window event handling as well as window updates.
        */
        void update();
        
        /**
         Creates and opens the window.
         
         @return
         True if the creation of the window was successful and false if not.
         */
        bool open();
        
        /**
         Closes the window.
         
         @remarks
         Destroys the SDL window in the process.
         */
        void close();
        
    private:
        SDL_Window* _SDLWindow;
        
        /**
         Handles the SDL events for the window.
         */
        void _handleEvents();
    };
}
