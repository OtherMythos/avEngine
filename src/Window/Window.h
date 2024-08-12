#pragma once

#include <string>
#include <vector>

#include "System/EnginePrerequisites.h"
#include "Input/InputPrerequisites.h"
#include "WindowPrerequisites.h"

namespace Ogre{
    class Window;
}

namespace AV{
    class InputManager;
    class InputMapper;
    class GuiInputProcessor;
    class GuiManager;

    class Window{
    protected:
        int _width;
        int _height;

        bool _initialised;
        bool _open;
        bool _minimized;
        bool _fullscreen;
        bool _borderless;

        std::string _currentTitle;

        Ogre::Window* _ogreWindow = 0;

        /**
        Get the default name of the window.
        */
        std::string	getDefaultWindowName() const;

    public:
        Window();
        virtual ~Window() = 0;
        virtual void update() = 0;
        virtual bool initialise() = 0;
        virtual bool open(InputManager* inputMan, GuiInputProcessor* guiManager) = 0;

        virtual bool close() = 0;

        virtual bool isInitialised() = 0;
        virtual bool isOpen() = 0;

        virtual InputMapper* getInputMapper() = 0;

        /**
         Transition the window to fullscreen.
         */
        virtual bool setFullscreen(FullscreenMode fullscreen) = 0;
        /**
         Transition the window to be borderless.
         */
        virtual bool setBorderless(bool fullscreen) = 0;

        virtual void setSize(int width, int height) = 0;

        virtual int getWindowDisplayIndex() = 0;

        /**
         Supply this window with an Ogre window.

         @remarks
         Ogre windows are actually quite abstract.
         You can use them to create a window on the screen, but you can also attach them to another window.
         As I'm using sdl for windowing and input, this is what I want to do. So the Ogre window infact doesn't act at all like a window.
         However, having it for reference is necessary to do the correct bookkeeping (resizing the ogre window when the actual window resizes, for example).

         I don't attempt to create the ogre window here because this windowing really has nothing to do with ogre.
         */
        virtual void injectOgreWindow(Ogre::Window *window) = 0;

        /**
         @return The width of the window.
         */
        int getWidth() const { return _width; };
        /**
         @return The height of the window.
         */
        int getHeight() const { return _height; };
        /**
         @return Whether the window was minimized.
         */
        bool getMinimized() const{ return _minimized; };
        void setMinimized(bool minimized) { _minimized = minimized; };

        virtual void setPosition(int x, int y) { }
        virtual int getPositionX() const { return 0; }
        virtual int getPositionY() const { return 0; }

        const std::string& getTitle() const { return _currentTitle; }
        virtual void setTitle(const std::string& title);

        virtual void rumbleInputDevice(InputDeviceId device, float lowFreqStrength, float highFreqStrength, uint32 rumbleTimeMs) = 0;

        enum class SystemCursor{
            CURSOR_ARROW = 0,
            CURSOR_IBEAM,
            CURSOR_WAIT,
            CURSOR_CROSSHAIR,
            CURSOR_WAITARROW,
            CURSOR_SIZENWSE,
            CURSOR_SIZENESW,
            CURSOR_SIZEWE,
            CURSOR_SIZENS,
            CURSOR_SIZEALL,
            CURSOR_NO,
            CURSOR_HAND,
            CURSOR_MAX
        };
        virtual void setSystemCursor(SystemCursor cursor);

        bool getFullscreen() const { return _fullscreen; };
        bool getBorderless() const { return _borderless; };

        typedef enum
        {
            MESSAGEBOX_ERROR                 = 0x00000010,
            MESSAGEBOX_WARNING               = 0x00000020,
            MESSAGEBOX_INFORMATION           = 0x00000040,
            MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT = 0x00000080,
            MESSAGEBOX_BUTTONS_RIGHT_TO_LEFT = 0x00000100
        } MessageBoxFlags;
        struct MessageBoxData{
            uint32 flags;
            std::string title;
            std::string message;
            std::vector<std::string> buttons;
            MessageBoxData() : flags(0), title("Title"), message("Message") { }
        };
        virtual bool showMessageBox(const MessageBoxData& msgData, int* pressedButton) = 0;

        virtual int getActualWidth() const;
        virtual int getActualHeight() const;
        /**
         @return The Ogre render window.
         */
        Ogre::Window* getRenderWindow() { return _ogreWindow; };

        //TODO Get rid of this!
        //It seems that in order to properly shut the engine down I need to destroy the root before closing the window.
        //I can do checks in the update as to whether to close, but it seems like renderOneFrame will hang if the window has already been destroyed.
        //So I need to notify the engine of when to close, which is what this is.
        //Rather than the window directly closing itself I need to tell it to close everything, which includes the window.
        //This should be done by the event system, so change this when the event system comes to be written.
        bool wantsToClose = false;
	};
}
