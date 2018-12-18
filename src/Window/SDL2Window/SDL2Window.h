#pragma once

#include "../Window.h"

#include "SDL_events.h"

struct SDL_Window;

namespace AV {
    class SDL2Window : public Window{
    public:
        SDL2Window();
        virtual ~SDL2Window();
        
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
         
         @return
         Returns true if the window closed successfully, false if not.
         
         @remarks
         Destroys the SDL window in the process.
         */
        bool close();
        
        bool isOpen();

    protected:
        SDL_Window* _SDLWindow;
        
        /**
         Polls the SDL events for the window.
         */
        void _pollForEvents();
        
        /**
         Process a single event.
         
         @param event
         The event to process.
         */
        void _handleEvent(SDL_Event &event);
        
        /**
         Update a single key event for either pressed or released.
         
         @param key
         The keycode in question.
         
         @param pressed
         Whether it was released or not.
         */
        void _handleKey(SDL_Keysym key, bool pressed);
    };
}
