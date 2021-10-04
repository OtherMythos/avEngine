#pragma once

#include "../Window.h"

//TODO might be able to get rid of this.
#include "OgreString.h"

namespace AV {
    class InputManager;
    class GuiManager;

    class iosWindow : public Window{
    public:
        iosWindow();
        virtual ~iosWindow();

        void update();

        bool open(InputManager* inputMan, GuiManager* guiManager);

        bool close();

        bool isOpen();

        Ogre::String getHandle();

        void injectOgreWindow(Ogre::Window *window);

        void showCursor(bool show);
        void grabCursor(bool capture);
        void warpMouseInWindow(int x, int y);

        InputMapper* getInputMapper(){return 0;};
        GuiInputProcessor* getGuiInputProcessor(){return 0;};

        //TODO remove duplication with this and SDL window.
        bool wantsToClose = false;

    };
}
