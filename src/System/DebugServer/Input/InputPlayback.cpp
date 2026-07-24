#ifdef DEBUG_SERVER

#include "InputPlayback.h"

#include "System/BaseSingleton.h"
#include "Input/InputManager.h"
#include "Window/GuiInputProcessor.h"
#include "Window/Window.h"

#include <algorithm>

namespace AV{
    InputPlayback::Result InputPlayback::applyButtonAction(const std::string& actionName, bool pressed, int frames){
        Result result;
        std::shared_ptr<InputManager> input = BaseSingleton::getInputManager();
        if(!input){
            result.error = "input manager not available";
            return result;
        }

        const ActionHandle handle = input->getButtonActionHandle(actionName);
        if(handle == INVALID_ACTION_HANDLE){
            result.error = "no button action named '" + actionName + "'";
            return result;
        }

        //A fresh press supersedes any existing hold of the same action.
        _dropMatching(Kind::ButtonAction, handle, 0);
        input->setButtonAction(SPOOF_DEVICE, handle, pressed);

        if(pressed && frames != 0){
            Spoof spoof;
            spoof.kind = Kind::ButtonAction;
            spoof.handle = handle;
            spoof.description = "button:" + actionName;
            spoof.framesRemaining = frames;
            mActive.push_back(spoof);
            result.releasesAtFrame = (frames < 0) ? -1 : static_cast<int64_t>(mFrameNumber) + frames;
        }

        result.ok = true;
        return result;
    }

    InputPlayback::Result InputPlayback::applyAxisAction(const std::string& actionName, float x, float y, int frames){
        Result result;
        std::shared_ptr<InputManager> input = BaseSingleton::getInputManager();
        if(!input){
            result.error = "input manager not available";
            return result;
        }

        const ActionHandle handle = input->getAxisActionHandle(actionName);
        if(handle == INVALID_ACTION_HANDLE){
            result.error = "no axis action named '" + actionName + "'";
            return result;
        }

        _dropMatching(Kind::AxisAction, handle, 0);
        input->setAxisAction(SPOOF_DEVICE, handle, true, x);
        input->setAxisAction(SPOOF_DEVICE, handle, false, y);

        const bool centred = (x == 0.0f && y == 0.0f);
        if(!centred && frames != 0){
            Spoof spoof;
            spoof.kind = Kind::AxisAction;
            spoof.handle = handle;
            spoof.description = "axis:" + actionName;
            spoof.framesRemaining = frames;
            mActive.push_back(spoof);
            result.releasesAtFrame = (frames < 0) ? -1 : static_cast<int64_t>(mFrameNumber) + frames;
        }

        result.ok = true;
        return result;
    }

    InputPlayback::Result InputPlayback::applyMouseButton(int button, bool pressed, int frames){
        Result result;
        if(button < 0 || button > 2){
            result.error = "mouse button must be 0 (left), 1 (right) or 2 (middle)";
            return result;
        }
        std::shared_ptr<InputManager> input = BaseSingleton::getInputManager();
        std::shared_ptr<GuiInputProcessor> gui = BaseSingleton::getGuiInputProcessor();
        if(!input || !gui){
            result.error = "input system not available";
            return result;
        }

        _dropMatching(Kind::MouseButton, 0, button);
        //Route through the GUI so hit-testing agrees, exactly as SDL2Window does.
        const bool intersectedGui = gui->processMouseButton(button, pressed);
        input->setMouseButton(button, pressed, intersectedGui);

        if(pressed && frames != 0){
            Spoof spoof;
            spoof.kind = Kind::MouseButton;
            spoof.button = button;
            spoof.description = "mouseButton:" + std::to_string(button);
            spoof.framesRemaining = frames;
            mActive.push_back(spoof);
            result.releasesAtFrame = (frames < 0) ? -1 : static_cast<int64_t>(mFrameNumber) + frames;
        }

        result.ok = true;
        return result;
    }

    InputPlayback::Result InputPlayback::applyMouseMove(float normX, float normY){
        Result result;
        std::shared_ptr<InputManager> input = BaseSingleton::getInputManager();
        std::shared_ptr<GuiInputProcessor> gui = BaseSingleton::getGuiInputProcessor();
        Window* window = BaseSingleton::getWindow();
        if(!input || !gui || !window){
            result.error = "input system not available";
            return result;
        }

        normX = std::max(0.0f, std::min(1.0f, normX));
        normY = std::max(0.0f, std::min(1.0f, normY));

        //processMouseMove takes normalised coordinates; the InputManager stores pixels.
        gui->processMouseMove(normX, normY);
        input->setMouseX(static_cast<int>(normX * window->getWidth()));
        input->setMouseY(static_cast<int>(normY * window->getHeight()));

        result.ok = true;
        return result;
    }

    void InputPlayback::clear(){
        for(const Spoof& spoof : mActive) _release(spoof);
        mActive.clear();
    }

    void InputPlayback::update(){
        mFrameNumber++;

        for(auto it = mActive.begin(); it != mActive.end();){
            if(it->framesRemaining < 0){
                ++it; //Indefinite hold.
                continue;
            }
            it->framesRemaining--;
            if(it->framesRemaining <= 0){
                _release(*it);
                it = mActive.erase(it);
            }else{
                ++it;
            }
        }
    }

    std::vector<InputPlayback::ActiveEntry> InputPlayback::getActive() const{
        std::vector<ActiveEntry> out;
        out.reserve(mActive.size());
        for(const Spoof& spoof : mActive){
            out.push_back({spoof.description, spoof.framesRemaining});
        }
        return out;
    }

    void InputPlayback::_release(const Spoof& spoof){
        std::shared_ptr<InputManager> input = BaseSingleton::getInputManager();
        if(!input) return;

        switch(spoof.kind){
            case Kind::ButtonAction:
                input->setButtonAction(SPOOF_DEVICE, spoof.handle, false);
                break;
            case Kind::AxisAction:
                input->setAxisAction(SPOOF_DEVICE, spoof.handle, true, 0.0f);
                input->setAxisAction(SPOOF_DEVICE, spoof.handle, false, 0.0f);
                break;
            case Kind::MouseButton:{
                std::shared_ptr<GuiInputProcessor> gui = BaseSingleton::getGuiInputProcessor();
                const bool intersectedGui = gui ? gui->processMouseButton(spoof.button, false) : false;
                input->setMouseButton(spoof.button, false, intersectedGui);
                break;
            }
        }
    }

    void InputPlayback::_dropMatching(Kind kind, ActionHandle handle, int button){
        for(auto it = mActive.begin(); it != mActive.end();){
            const bool match = it->kind == kind &&
                (kind == Kind::MouseButton ? it->button == button : it->handle == handle);
            if(match) it = mActive.erase(it);
            else ++it;
        }
    }
}

#endif
