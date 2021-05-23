#include "GuiInputProcessor.h"

#include "Logger/Log.h"

#include "InputMapper.h"
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

    static inline void _performGuiAction(Colibri::ColibriManager* colibriManager, GuiInputTypes type, bool pressed){
        if(type == GuiInputTypes::Primary){
            if(pressed) colibriManager->setKeyboardPrimaryPressed();
            else colibriManager->setKeyboardPrimaryReleased();
            return;
        }

        void(Colibri::ColibriManager::*funcPtr)(Colibri::Borders::Borders) = &Colibri::ColibriManager::setKeyDirectionPressed;
        if(!pressed) funcPtr = &Colibri::ColibriManager::setKeyDirectionReleased;

        static const Colibri::Borders::Borders borders[] = {
            //Take the place of None
            Colibri::Borders::Top,

            Colibri::Borders::Top,
            Colibri::Borders::Left,
            Colibri::Borders::Right,
            Colibri::Borders::Bottom,
        };
        assert(static_cast<int>(type) < sizeof(borders)/sizeof(Colibri::Borders::Borders) );

        (colibriManager->*funcPtr)(borders[ static_cast<int>(type) ]);
    }

    void GuiInputProcessor::processControllerButton(const InputMapper& mapper, bool pressed, int button){
        GuiInputTypes type = mapper.getGuiActionForButton(button);
        if(type == GuiInputTypes::None) return;

        Colibri::ColibriManager* colibriManager = mGuiManager->getColibriManager();
        _performGuiAction(colibriManager, type, pressed);
    }

    void GuiInputProcessor::processInputKey(const InputMapper& mapper, bool pressed, int key, int keyMod, bool textInputEnabled){
        Colibri::ColibriManager* colibriManager = mGuiManager->getColibriManager();

        if(textInputEnabled){
            if(pressed) colibriManager->setTextSpecialKeyPressed(key, keyMod);
            else colibriManager->setTextSpecialKeyReleased(key, keyMod);
        }

        GuiInputTypes type = mapper.getGuiActionForKey(key);
        if(type == GuiInputTypes::None) return;
        _performGuiAction(colibriManager, type, pressed);
    }
}
