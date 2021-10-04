#pragma once

#include <string>

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

        bool _open;

        Ogre::Window* _ogreWindow = 0;

        /**
        Get the default name of the window.
        */
        std::string	getDefaultWindowName() const;

    public:
        Window();
        virtual ~Window() = 0;
        virtual void update() = 0;
        virtual bool open(InputManager* inputMan, GuiManager* guiManager) = 0;

        virtual bool close() = 0;

        virtual bool isOpen() = 0;

        virtual InputMapper* getInputMapper() = 0;
        virtual GuiInputProcessor* getGuiInputProcessor() = 0;

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
