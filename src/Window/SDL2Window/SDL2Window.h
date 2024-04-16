#pragma once

#include "Window/Window.h"

#include "SDL_events.h"
#include "OgreString.h"
#include "Input/InputPrerequisites.h"
#include "SDL2InputMapper.h"
#include "Window/GuiInputProcessor.h"

struct SDL_Window;
struct SDL_SysWMinfo;

namespace AV {
    class InputManager;
    class GuiManager;

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
        bool open(InputManager* inputMan, GuiInputProcessor* guiInputProcessor);

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
        Ogre::String getX11Handle(SDL_SysWMinfo* wmInfo);
        /**
         Setup any platform specific Ogre features.
         */
        void extraOgreSetup();

        /**
         Inject the sdl window with an ogre window.
         */
        void injectOgreWindow(Ogre::Window *window);

        void showCursor(bool show);
        void grabCursor(bool capture);
        void warpMouseInWindow(int x, int y);

        int getActualWidth() const;
        int getActualHeight() const;

        void setPosition(int x, int y) override;
        int getPositionX() const override;
        int getPositionY() const override;

        bool setFullscreen(bool fullscreen) override;

        bool setBorderless(bool enable) override;

        void setSize(int width, int height) override;

        void setTitle(const std::string& title) override;

        bool showMessageBox(const MessageBoxData& msgData, int* pressedButton) override;

        void setSystemCursor(SystemCursor cursor) override;

        void rumbleInputDevice(InputDeviceId device, float lowFreqStrength, float highFreqStrength, uint32 rumbleTimeMs);

    protected:
        SDL_Window* _SDLWindow;
        struct ControllerEntry{
            SDL_GameController* controller;
            InputDeviceId controllerId;
        };
        ControllerEntry mOpenGameControllers[MAX_INPUT_DEVICES];

        SDL2InputMapper inputMapper;
        GuiInputProcessor* mGuiInputProcessor;
        InputManager* mInputManager;

        //Will be true when the user has selected some sort of text input and regular keyboard input is disabled as a result of it.
        bool isKeyboardInputEnabled = false;

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
        static int _handleAppEvents(void *userdata, SDL_Event *event);
        void _handleBasicWindowEvent(const SDL_WindowEvent& event);

        /**
         Update a single key event for either pressed or released.

         @param key
         The keycode in question.

         @param pressed
         Whether it was released or not.
         */
        void _handleKey(SDL_Keysym key, bool pressed);

        void _handleMouseButton(int button, bool pressed);
        void _handleMouseMotion(float x, float y);

        void _handleControllerAxis(const SDL_Event& e);
        void _handleControllerButton(const SDL_Event& e);
        void _handleDeviceChange(const SDL_Event& e);
        void _handleJoystickAddition(const SDL_Event& e);

        /**
        Add a controller device.
        This function was separated from the handle device change as it seems that windows is a bit flaky about registering the new controllers on startup.
        Instead I have to run this in a loop rather than wait for the events.
        It should only be called as part of the add controller sdl event procedure.
        */
        void _addController(InputDeviceId i);

        std::vector<InputDeviceId> mRegisteredDevices;

        InputMapper* getInputMapper(){
            return &inputMapper;
        }

        /**
        Resize the window and the ogre window based off a resize event.

        @param event
        An SDL_WINDOWEVENT_RESIZED event. The argument needs to be this exact event type otherwise the function will return.
        */
        void _resizeWindow(SDL_Event &event);

        void _destroySystemCursors();
        void _setupSystemCursors();

    private:
        bool mResetInputsAtFrameEnd;

        static const int NUM_SYSTEM_CURSORS = 12;
        SDL_Cursor* mSystemCursors[NUM_SYSTEM_CURSORS];

        void _setSize(int width, int height);
    };
}
