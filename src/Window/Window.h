#pragma once

namespace Ogre{
    class RenderWindow;
}

namespace AV{
    class Window{
    protected:
        int _width;
        int _height;

        bool _open;

        Ogre::RenderWindow* _ogreWindow = 0;

    public:
        Window();
        virtual ~Window() = 0;
        virtual void update() = 0;
        virtual bool open() = 0;

        virtual bool close() = 0;

        virtual bool isOpen() = 0;

        /**
         Supply this window with an Ogre window.

         @remarks
         Ogre windows are actually quite abstract.
         You can use them to create a window on the screen, but you can also attach them to another window.
         As I'm using sdl for windowing and input, this is what I want to do. So the Ogre window infact doesn't act at all like a window.
         However, having it for reference is necessary to do the correct bookkeeping (resizing the ogre window when the actual window resizes, for example).

         I don't attempt to create the ogre window here because this windowing really has nothing to do with ogre.
         */
        virtual void injectOgreWindow(Ogre::RenderWindow *window) = 0;

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
        Ogre::RenderWindow* getRenderWindow() { return _ogreWindow; };
	};
}
