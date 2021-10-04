#include "iosWindow.h"


namespace AV{
    iosWindow::iosWindow(){

    }

    iosWindow::~iosWindow(){

    }


    void iosWindow::update(){

    }

    bool iosWindow::open(InputManager* inputMan, GuiManager* guiManager){
        return false;
    }

    bool iosWindow::close(){
        return false;
    }

    bool iosWindow::isOpen(){
        return true;
    }

    void iosWindow::injectOgreWindow(Ogre::Window *window){
        _ogreWindow = window;
    }

    Ogre::String iosWindow::getHandle() { return ""; }

}
