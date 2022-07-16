#include "GuiInputProcessor.h"

#include "Logger/Log.h"

#include "InputMapper.h"
#include "ColibriGui/ColibriManager.h"
#include "Gui/GuiManager.h"

namespace AV{
    GuiInputProcessor::GuiInputProcessor()
        : mAxisX(0), mAxisY(0), mCurrentControllerGuiType(GuiInputTypes::None) {

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

        const Ogre::Vector2 mouseVal(Ogre::Vector2(x, y) * colibriManager->getCanvasSize());
        mGuiManager->setGuiMousePos(mouseVal);
        colibriManager->setMouseCursorMoved(mouseVal);
    }

    void GuiInputProcessor::processMouseScroll(float x, float y){
        Colibri::ColibriManager* colibriManager = mGuiManager->getColibriManager();
        const float scroll = mGuiManager->getMouseScrollSpeed();

        colibriManager->setScroll(Ogre::Vector2(x * scroll, y * scroll), false);
    }

    bool GuiInputProcessor::processMouseButton(int mouseButton, bool pressed){
        if(mouseButton != 0) return false; //If not the left button, colibri only supports that.
        Colibri::ColibriManager* colibriManager = mGuiManager->getColibriManager();

        bool guiConsumed = false;
        //It seems that Colibri only supports a single mouse button right now.
        if(pressed){
            guiConsumed = colibriManager->isMouseCursorFocusedOnWidget();
            colibriManager->setMouseCursorPressed(true, false);
        }
        else colibriManager->setMouseCursorReleased();

        return guiConsumed;
    }

    void GuiInputProcessor::processWindowResize(int width, int height){
        Colibri::ColibriManager* colibriManager = mGuiManager->getColibriManager();
        colibriManager->setCanvasSize(Ogre::Vector2(1920.0f, 1080.0f), Ogre::Vector2(width, height));
    }

    void GuiInputProcessor::processTextInput(const char* text){
        Colibri::ColibriManager* colibriManager = mGuiManager->getColibriManager();
        colibriManager->setTextInput(text, false);
    }

    void GuiInputProcessor::processTextEdit(const char *text, int selectStart, int selectLength){
        Colibri::ColibriManager* colibriManager = mGuiManager->getColibriManager();
        colibriManager->setTextEdit(text, selectStart, selectLength);
    }

    static const Colibri::Borders::Borders borders[] = {
        //Take the place of None
        Colibri::Borders::Top,

        Colibri::Borders::Top,
        Colibri::Borders::Left,
        Colibri::Borders::Right,
        Colibri::Borders::Bottom,
    };
    static inline void _performGuiAction(Colibri::ColibriManager* colibriManager, GuiInputTypes type, bool pressed){
        if(type == GuiInputTypes::Primary){
            if(pressed) colibriManager->setKeyboardPrimaryPressed();
            else colibriManager->setKeyboardPrimaryReleased();
            return;
        }

        void(Colibri::ColibriManager::*funcPtr)(Colibri::Borders::Borders) = &Colibri::ColibriManager::setKeyDirectionPressed;
        if(!pressed) funcPtr = &Colibri::ColibriManager::setKeyDirectionReleased;

        assert(static_cast<int>(type) < sizeof(borders)/sizeof(Colibri::Borders::Borders) );

        (colibriManager->*funcPtr)(borders[ static_cast<int>(type) ]);
    }

    void GuiInputProcessor::processControllerAxis(const InputMapper& mapper, int axisId, float value, bool xAxis){
        GuiMappedAxisData outData;
        bool found = mapper.getGuiActionForAxis(axisId, &outData);
        if(!found) return;

        assert(
            outData.bottom != GuiInputTypes::None &&
            outData.top != GuiInputTypes::None &&
            outData.left != GuiInputTypes::None &&
            outData.right != GuiInputTypes::None
        );

        //Not enough movement occured to warrant a re-check.
        if(value > -0.3 && value < 0.3){
            if(mCurrentControllerGuiType != GuiInputTypes::None){
                mGuiManager->getColibriManager()->setKeyDirectionReleased(borders[ static_cast<int>(mCurrentControllerGuiType) ]);
            }
            mCurrentControllerGuiType = GuiInputTypes::None;
            mAxisX = 0;
            mAxisY = 0;
            return;
        }

        if(xAxis) mAxisX = value;
        else mAxisY = value;

        float angle = atan2(mAxisX, -mAxisY);
        float degree = angle * 180 / 3.14159265359f;
        degree += 180.0f;
        //std::cout << degree << std::endl;

        Colibri::ColibriManager* colibriManager = mGuiManager->getColibriManager();
        GuiInputTypes guiInputType = GuiInputTypes::None;

        if(degree > 45.0f && degree < 135.0f) guiInputType = outData.left;
        else if(degree > 135.0f && degree < 225.0f) guiInputType = outData.top;
        else if(degree > 225.0f && degree < 315.0f) guiInputType = outData.right;
        else if(degree < 45.0f || degree > 315.0f) guiInputType = outData.bottom;

        // const char* desc = 0;
        // switch(guiInputType){
        //     case GuiInputTypes::Bottom: desc = "bottom"; break;
        //     case GuiInputTypes::Top: desc = "top"; break;
        //     case GuiInputTypes::Left: desc = "left"; break;
        //     case GuiInputTypes::Right: desc = "right"; break;
        //     default: desc = "none"; break;
        // }
        // std::cout << desc << std::endl;

        if(mCurrentControllerGuiType != guiInputType){
            colibriManager->setKeyDirectionReleased(borders[ static_cast<int>(mCurrentControllerGuiType) ]);
            colibriManager->setKeyDirectionPressed(borders[ static_cast<int>(guiInputType) ]);
            mCurrentControllerGuiType = guiInputType;
        }
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
