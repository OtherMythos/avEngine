#pragma once

#include "../Window.h"

#include "SDL_events.h"
#include "OgreString.h"
#include "Input/InputPrerequisites.h"
#include "SDL2InputMapper.h"

struct SDL_Window;

namespace AV {
    class InputManager;

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
        bool open(InputManager* inputMan);

        /**
         Closes the window.

         @return
         Returns true if the window closed successfully, false if not.

         @remarks
         Destroys the SDL window in the process.
         */
        bool close();

        bool isOpen();

        /**
         Get the handle of this window from sdl.

         @return
         The handle if the window can be queried successfully. 0 if not.
         */
        Ogre::String getHandle();

        /**
         Inject the sdl window with an ogre window.
         */
        void injectOgreWindow(Ogre::RenderWindow *window);

        void grabCursor(bool capture);


        //TODO Get rid of this!
        //It seems that in order to properly shut the engine down I need to destroy the root before closing the window.
        //I can do checks in the update as to whether to close, but it seems like renderOneFrame will hang if the window has already been destroyed.
        //So I need to notify the engine of when to close, which is what this is.
        //Rather than the window directly closing itself I need to tell it to close everything, which includes the window.
        //This should be done by the event system, so change this when the event system comes to be written.
        bool wantsToClose = false;

    protected:
        SDL_Window* _SDLWindow;
        struct ControllerEntry{
            SDL_GameController* controller;
            unsigned char controllerId;
        };
        ControllerEntry mOpenGameControllers[MAX_INPUT_DEVICES];

        SDL2InputMapper inputMapper;
        InputManager* mInputManager;

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

        void _handleMouseButton(int button, bool pressed);

        void _handleControllerAxis(const SDL_Event& e);
        void _handleControllerButton(const SDL_Event& e);
        void _handleDeviceChange(const SDL_Event& e);

        /**
        Resize the window and the ogre window based off a resize event.

        @param event
        An SDL_WINDOWEVENT_RESIZED event. The argument needs to be this exact event type otherwise the function will return.
        */
        void _resizeWindow(SDL_Event &event);
    };
}
