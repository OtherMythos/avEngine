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

    bool GuiInputProcessor::shouldTextInputEnable(){
        Colibri::ColibriManager* colibriManager = mGuiManager->getColibriManager();
        return colibriManager->focusedWantsTextInput();
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

    void GuiInputProcessor::processTextInput(const char* text){
        Colibri::ColibriManager* colibriManager = mGuiManager->getColibriManager();
        colibriManager->setTextInput(text);
    }

    void GuiInputProcessor::processTextEdit(const char *text, int selectStart, int selectLength){
        Colibri::ColibriManager* colibriManager = mGuiManager->getColibriManager();
        colibriManager->setTextEdit(text, selectStart, selectLength);
    }

    void GuiInputProcessor::processInputKey(bool pressed, int key, int keyMod, bool textInputEnabled){
        Colibri::ColibriManager* colibriManager = mGuiManager->getColibriManager();


        void(Colibri::ColibriManager::*funcPtr)(Colibri::Borders::Borders) = &Colibri::ColibriManager::setKeyDirectionPressed;
        if(!pressed) funcPtr = &Colibri::ColibriManager::setKeyDirectionReleased;

        if(key == 82){
            (colibriManager->*funcPtr)(Colibri::Borders::Top);
        }else if(key == 81){
            (colibriManager->*funcPtr)(Colibri::Borders::Bottom);
        }else if(key == 80){
            (colibriManager->*funcPtr)(Colibri::Borders::Left);
        }else if(key == 79){
            (colibriManager->*funcPtr)(Colibri::Borders::Right);
        }
        else if(key == 13){ //Return
            if(pressed) colibriManager->setKeyboardPrimaryPressed();
            else colibriManager->setKeyboardPrimaryReleased();
        }

        if(textInputEnabled){
            if(pressed) colibriManager->setTextSpecialKeyPressed(key, keyMod);
            else colibriManager->setTextSpecialKeyReleased(key, keyMod);
        }

    }
}
