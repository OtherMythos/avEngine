#include "GuiInputProcessor.h"

#include "Logger/Log.h"

#include "ColibriGui/ColibriManager.h"
#include "Gui/GuiManager.h"

namespace AV{
    GuiInputProcessor::GuiInputProcessor(){

    }

    GuiInputProcessor::~GuiInputProcessor(){

    }

    void GuiInputProcessor::initialise(GuiManager* guiManager){
        mGuiManager = guiManager;
    }

    void GuiInputProcessor::processMouseMove(float x, float y){
        Colibri::ColibriManager* colibriManager = mGuiManager->getColibriManager();

        colibriManager->setMouseCursorMoved(Ogre::Vector2(x, y) * colibriManager->getCanvasSize());
    }

    void GuiInputProcessor::processMouseButton(int mouseButton, bool pressed){
        if(mouseButton != 0) return; //If not the left button.
        Colibri::ColibriManager* colibriManager = mGuiManager->getColibriManager();

        //It seems that Colibri only supports a single mouse button right now.
        if(pressed) colibriManager->setMouseCursorPressed(true, false);
        else colibriManager->setMouseCursorReleased();
    }

    void GuiInputProcessor::processWindowResize(int width, int height){
        Colibri::ColibriManager* colibriManager = mGuiManager->getColibriManager();
        colibriManager->setCanvasSize(Ogre::Vector2(1920.0f, 1080.0f), Ogre::Vector2(width, height));
    }
}
