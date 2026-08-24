#include "InputRouter.h"

#include "InputManager.h"
#include "Window/GuiInputProcessor.h"
#include "Window/InputMapper.h"
#include "Window/Window.h"

#include "Event/EventDispatcher.h"
#include "Event/Events/DebuggerToolEvent.h"

#include <algorithm>
#include <cassert>
#include <cstring>

namespace AV{

    InputRouter::InputRouter()
        : mNextHandle(1),
          mPointerFocus(INVALID_INPUT_LAYER),
          mLastMouseX(0.0f),
          mLastMouseY(0.0f),
          mInputManager(0),
          mGuiInputProcessor(0),
          mWindow(0),
          mExternalLayersEnabled(true),
          mGuiConsumes(false) {

        for(int i = 0; i < NUM_ROUTED_MOUSE_BUTTONS; i++){
            mMouseButtonOwner[i] = INVALID_INPUT_LAYER;
        }
    }

    InputRouter::~InputRouter(){

    }

    void InputRouter::initialise(InputManager* inputManager, GuiInputProcessor* guiInputProcessor, Window* window){
        mInputManager = inputManager;
        mGuiInputProcessor = guiInputProcessor;
        mWindow = window;

        //The engine's own layers. These are dispatched by kind rather than
        //through the callback struct, so they're registered with an empty one.
        InputLayerCallbacks empty;
        memset(&empty, 0, sizeof(InputLayerCallbacks));

        mLayers.clear();
        mLayers.push_back({mNextHandle++, "engineHotkeys", INPUT_PRIORITY_SYSTEM, BuiltinKind::SystemHotkeys, true, empty, 0});
        mLayers.push_back({mNextHandle++, "gui", INPUT_PRIORITY_GUI, BuiltinKind::Gui, true, empty, 0});
        mLayers.push_back({mNextHandle++, "game", INPUT_PRIORITY_GAME, BuiltinKind::GameSink, true, empty, 0});
    }

    InputLayerHandle InputRouter::addLayer(const char* name, int priority, const InputLayerCallbacks& callbacks, void* userData){
        Layer l;
        l.handle = mNextHandle++;
        l.name = name;
        l.priority = priority;
        l.kind = BuiltinKind::External;
        l.enabled = true;
        l.cb = callbacks;
        l.userData = userData;

        //Insert ahead of the first layer of lower priority, so layers which
        //share a priority stay in registration order.
        auto it = std::find_if(mLayers.begin(), mLayers.end(), [priority](const Layer& other){
            return other.priority < priority;
        });
        mLayers.insert(it, l);

        return l.handle;
    }

    void InputRouter::removeLayer(InputLayerHandle handle){
        auto it = std::find_if(mLayers.begin(), mLayers.end(), [handle](const Layer& l){
            return l.handle == handle;
        });
        if(it == mLayers.end()) return;
        //Builtin layers live as long as the router does.
        if(it->kind != BuiltinKind::External) return;

        const bool heldPointer = (mPointerFocus == handle);

        _notifyCancelled(*it);
        _clearOwnershipFor(handle);
        mLayers.erase(it);

        if(heldPointer){
            mPointerFocus = INVALID_INPUT_LAYER;
            //Replay the last known position so whichever layer inherits the
            //pointer has correct hover state now, rather than whenever the
            //mouse next happens to move.
            injectMouseMove(mLastMouseX, mLastMouseY);
        }
    }

    void InputRouter::setLayerEnabled(InputLayerHandle handle, bool enabled){
        Layer* l = _findLayer(handle);
        if(!l || l->enabled == enabled) return;

        l->enabled = enabled;
        if(enabled) return;

        const bool heldPointer = (mPointerFocus == handle);
        _notifyCancelled(*l);
        _clearOwnershipFor(handle);
        if(heldPointer){
            mPointerFocus = INVALID_INPUT_LAYER;
            injectMouseMove(mLastMouseX, mLastMouseY);
        }
    }

    bool InputRouter::isLayerEnabled(InputLayerHandle handle) const{
        const Layer* l = _findLayer(handle);
        return l ? l->enabled : false;
    }

    void InputRouter::setExternalLayersEnabled(bool enabled){
        if(mExternalLayersEnabled == enabled) return;
        mExternalLayersEnabled = enabled;

        if(enabled) return;
        //Revoke anything the external layers were holding, otherwise they keep
        //ownership of buttons they can no longer be told about.
        for(const Layer& l : mLayers){
            if(l.kind != BuiltinKind::External) continue;
            _notifyCancelled(l);
            _clearOwnershipFor(l.handle);
        }
        mPointerFocus = INVALID_INPUT_LAYER;
    }

    InputRouter::Layer* InputRouter::_findLayer(InputLayerHandle handle){
        for(Layer& l : mLayers){
            if(l.handle == handle) return &l;
        }
        return 0;
    }

    const InputRouter::Layer* InputRouter::_findLayer(InputLayerHandle handle) const{
        for(const Layer& l : mLayers){
            if(l.handle == handle) return &l;
        }
        return 0;
    }

    bool InputRouter::_layerIsLive(const Layer& l) const{
        if(l.kind == BuiltinKind::Gui) return mGuiInputProcessor != 0;
        if(l.kind == BuiltinKind::GameSink) return mInputManager != 0;
        if(l.kind == BuiltinKind::SystemHotkeys) return true;

        if(!l.cb.isLive) return true;
        return l.cb.isLive(l.userData);
    }

    bool InputRouter::_layerAcceptsEvents(const Layer& l) const{
        if(!l.enabled) return false;
        if(l.kind == BuiltinKind::External && !mExternalLayersEnabled) return false;
        return _layerIsLive(l);
    }

    void InputRouter::_normalise(float x, float y, float* outX, float* outY) const{
        const float w = (mWindow && mWindow->getWidth() > 0) ? (float)mWindow->getWidth() : 1.0f;
        const float h = (mWindow && mWindow->getHeight() > 0) ? (float)mWindow->getHeight() : 1.0f;
        *outX = x / w;
        *outY = y / h;
    }

    InputLayerHandle InputRouter::_heldButtonOwner() const{
        for(int i = 0; i < NUM_ROUTED_MOUSE_BUTTONS; i++){
            if(mMouseButtonOwner[i] != INVALID_INPUT_LAYER) return mMouseButtonOwner[i];
        }
        return INVALID_INPUT_LAYER;
    }

    void InputRouter::_clearOwnershipFor(InputLayerHandle handle){
        for(int i = 0; i < NUM_ROUTED_MOUSE_BUTTONS; i++){
            if(mMouseButtonOwner[i] == handle) mMouseButtonOwner[i] = INVALID_INPUT_LAYER;
        }
        for(auto it = mKeyOwner.begin(); it != mKeyOwner.end(); ){
            if(it->second == handle) it = mKeyOwner.erase(it);
            else ++it;
        }
    }

    void InputRouter::_notifyCancelled(const Layer& l){
        if(l.kind == BuiltinKind::External){
            if(l.cb.onInputCancelled) l.cb.onInputCancelled(l.userData);
            return;
        }
        if(l.kind == BuiltinKind::GameSink && mInputManager){
            for(int i = 0; i < NUM_ROUTED_MOUSE_BUTTONS; i++){
                mInputManager->setMouseButton(i, false, false);
            }
        }
    }

    void InputRouter::_notifyPointerFocusLost(const Layer& l){
        if(l.kind == BuiltinKind::External){
            if(l.cb.onPointerFocusLost) l.cb.onPointerFocusLost(l.userData);
            return;
        }
        if(l.kind == BuiltinKind::Gui && mGuiInputProcessor){
            //Park the cursor away from the canvas so any hover highlight clears
            //and a later hit test can't claim a widget the user isn't over.
            mGuiInputProcessor->parkMouseCursor();
        }
    }

    bool InputRouter::_dispatchMouseButton(const Layer& l, int button, bool pressed){
        switch(l.kind){
            case BuiltinKind::External:
                if(!l.cb.onMouseButton) return false;
                return l.cb.onMouseButton(button, pressed, l.userData);
            case BuiltinKind::Gui:{
                if(!mGuiInputProcessor) return false;
                const bool intersected = mGuiInputProcessor->processMouseButton(button, pressed);
                //The gui always reports what it intersected, but only blocks the
                //layers below when the project has asked it to.
                if(intersected && mInputManager) mInputManager->setMouseGuiIntersected(true);
                return intersected && mGuiConsumes;
            }
            case BuiltinKind::GameSink:
                if(!mInputManager) return false;
                mInputManager->setMouseButton(button, pressed, mInputManager->getMouseGuiIntersected());
                return false;
            case BuiltinKind::SystemHotkeys:
            default:
                return false;
        }
    }

    bool InputRouter::_dispatchKey(const Layer& l, int scancode, int keycode, int keyMod, bool pressed){
        switch(l.kind){
            case BuiltinKind::External:
                if(!l.cb.onKey) return false;
                return l.cb.onKey(scancode, keycode, keyMod, pressed, l.userData);
            case BuiltinKind::SystemHotkeys:
                return _dispatchHotkey(scancode, pressed);
            case BuiltinKind::Gui:{
                if(!mGuiInputProcessor || !mWindow) return false;
                InputMapper* mapper = mWindow->getInputMapper();
                if(!mapper) return false;
                //Colibri's own text state, not the router wide answer: another
                //layer wanting text input says nothing about whether Colibri has
                //a focused text widget to send special keys to.
                mGuiInputProcessor->processInputKey(*mapper, pressed, scancode, keycode, keyMod, mGuiInputProcessor->shouldTextInputEnable());
                return false;
            }
            case BuiltinKind::GameSink:{
                if(!mInputManager) return false;
                if(mWindow){
                    InputMapper* mapper = mWindow->getInputMapper();
                    if(mapper){
                        const ActionHandle handle = mapper->getKeyboardMap(keycode);
                        mInputManager->setKeyboardKeyAction(handle, pressed ? 1.0f : 0.0f);
                    }
                }
                mInputManager->setKeyboardInput(scancode, pressed);
                return false;
            }
            default:
                return false;
        }
    }

    bool InputRouter::_dispatchHotkey(int scancode, bool pressed){
#ifdef DEBUGGING_TOOLS
        if(!pressed) return false;

        DebuggerToolToggle toggle;
        if(scancode == INPUT_SCANCODE_F1) toggle = DebuggerToolToggle::StatsToggle;
        else if(scancode == INPUT_SCANCODE_F2) toggle = DebuggerToolToggle::MeshesToggle;
        else return false;

        DebuggerToolEventToggle event;
        event.t = toggle;
        EventDispatcher::transmitEvent(EventType::DebuggerTools, event);
        //Consumed, so a debug overlay below can't also act on the same key.
        return true;
#else
        return false;
#endif
    }

    bool InputRouter::_dispatchWheel(const Layer& l, float x, float y){
        switch(l.kind){
            case BuiltinKind::External:
                if(!l.cb.onMouseWheel) return false;
                return l.cb.onMouseWheel(x, y, l.userData);
            case BuiltinKind::Gui:
                if(!mGuiInputProcessor) return false;
                mGuiInputProcessor->processMouseScroll(x, y * 2.0f);
                return false;
            case BuiltinKind::GameSink:
                if(!mInputManager) return false;
                mInputManager->setMouseWheel((int)y);
                return false;
            default:
                return false;
        }
    }

    void InputRouter::_dispatchMouseMove(const Layer& l, float x, float y, float normX, float normY, bool focused){
        switch(l.kind){
            case BuiltinKind::External:
                if(l.cb.onMouseMove) l.cb.onMouseMove(x, y, normX, normY, focused, l.userData);
                return;
            case BuiltinKind::Gui:
                //Only tracked while focused. The park on losing focus is what
                //stops a stale highlight being left behind.
                if(focused && mGuiInputProcessor) mGuiInputProcessor->processMouseMove(normX, normY);
                return;
            case BuiltinKind::GameSink:{
                if(!mInputManager) return;
                //Position is stored regardless of who owns the pointer, so
                //nothing downstream ever sees the cursor freeze.
                mInputManager->setMouseX((int)x);
                mInputManager->setMouseY((int)y);
                if(mWindow){
                    const float actualWidth = mWindow->getWidth() > 0 ? (float)mWindow->getActualWidth() / (float)mWindow->getWidth() : 1.0f;
                    const float actualHeight = mWindow->getHeight() > 0 ? (float)mWindow->getActualHeight() / (float)mWindow->getHeight() : 1.0f;
                    mInputManager->setActualMouseX((int)(x * actualWidth));
                    mInputManager->setActualMouseY((int)(y * actualHeight));
                }
                return;
            }
            default:
                return;
        }
    }

    InputLayerHandle InputRouter::_resolvePointerFocus(){
        //A layer holding a mouse button keeps the pointer for the whole drag,
        //whatever anything above it would otherwise claim. Without this a fast
        //drag across another layer's widget hands focus away mid-gesture.
        InputLayerHandle newFocus = _heldButtonOwner();

        if(newFocus == INVALID_INPUT_LAYER){
            for(const Layer& l : mLayers){
                if(!_layerAcceptsEvents(l)) continue;
                if(l.kind != BuiltinKind::External) continue;
                if(!l.cb.wantsPointer) continue;
                if(l.cb.wantsPointer(l.userData)){
                    newFocus = l.handle;
                    break;
                }
            }
        }
        //Nothing claimed it, so the gui tracks the pointer as it always has.
        if(newFocus == INVALID_INPUT_LAYER){
            for(const Layer& l : mLayers){
                if(l.kind == BuiltinKind::Gui && _layerAcceptsEvents(l)){
                    newFocus = l.handle;
                    break;
                }
            }
        }

        if(newFocus != mPointerFocus){
            const Layer* old = _findLayer(mPointerFocus);
            if(old) _notifyPointerFocusLost(*old);
            mPointerFocus = newFocus;
        }
        return mPointerFocus;
    }

    void InputRouter::injectMouseMove(float x, float y){
        mLastMouseX = x;
        mLastMouseY = y;

        float normX, normY;
        _normalise(x, y, &normX, &normY);

        const InputLayerHandle focus = _resolvePointerFocus();

        //Motion is broadcast rather than arbitrated, so iterate a copy: a layer
        //could remove itself while being told about it.
        const std::vector<Layer> layers = mLayers;
        for(const Layer& l : layers){
            if(!_layerAcceptsEvents(l)) continue;
            _dispatchMouseMove(l, x, y, normX, normY, l.handle == focus);
        }
    }

    void InputRouter::injectMouseButton(int button, bool pressed){
        if(button < 0 || button >= NUM_ROUTED_MOUSE_BUTTONS) return;

        InputLayerHandle& owner = mMouseButtonOwner[button];

        if(pressed){
            //A press while already held can only come from injected input. Pair
            //off the outstanding press first so ownership can't leak.
            if(owner != INVALID_INPUT_LAYER){
                const Layer* current = _findLayer(owner);
                if(current) _dispatchMouseButton(*current, button, false);
                owner = INVALID_INPUT_LAYER;
            }

            const std::vector<Layer> layers = mLayers;
            for(const Layer& l : layers){
                if(!_layerAcceptsEvents(l)) continue;
                if(_dispatchMouseButton(l, button, true)){
                    owner = l.handle;
                    //A press taken by anything above the game counts as having
                    //landed on ui, which is what the touch events report.
                    if(l.kind != BuiltinKind::GameSink && mInputManager){
                        mInputManager->setMouseGuiIntersected(true);
                    }
                    break;
                }
            }
            return;
        }

        if(owner != INVALID_INPUT_LAYER){
            //Routed to the owner alone, and deliberately without the liveness
            //check: a layer which went quiet mid-drag still has to be told the
            //button came up. Lower layers never saw the press, so they get
            //nothing here.
            const Layer* current = _findLayer(owner);
            if(current) _dispatchMouseButton(*current, button, false);
            owner = INVALID_INPUT_LAYER;
            return;
        }

        //An unowned release is broadcast and never consumed, so a layer can't
        //eat the release half of a press some lower layer is still holding.
        const std::vector<Layer> layers = mLayers;
        for(const Layer& l : layers){
            if(!_layerAcceptsEvents(l)) continue;
            _dispatchMouseButton(l, button, false);
        }
    }

    void InputRouter::injectMouseWheel(float x, float y){
        //Offered to whoever owns the pointer first, as that's the layer the
        //user is looking at.
        if(mPointerFocus != INVALID_INPUT_LAYER){
            const Layer* focused = _findLayer(mPointerFocus);
            if(focused && _layerAcceptsEvents(*focused)){
                if(_dispatchWheel(*focused, x, y)) return;
            }
        }

        const std::vector<Layer> layers = mLayers;
        for(const Layer& l : layers){
            if(!_layerAcceptsEvents(l)) continue;
            if(l.handle == mPointerFocus) continue; //Already offered above.
            if(_dispatchWheel(l, x, y)) return;
        }
    }

    void InputRouter::injectKey(int scancode, int keycode, int keyMod, bool pressed){
        auto ownerIt = mKeyOwner.find(scancode);

        if(pressed){
            if(ownerIt != mKeyOwner.end()){
                const Layer* current = _findLayer(ownerIt->second);
                if(current) _dispatchKey(*current, scancode, keycode, keyMod, false);
                mKeyOwner.erase(ownerIt);
            }

            const std::vector<Layer> layers = mLayers;
            for(const Layer& l : layers){
                if(!_layerAcceptsEvents(l)) continue;
                if(_dispatchKey(l, scancode, keycode, keyMod, true)){
                    mKeyOwner[scancode] = l.handle;
                    break;
                }
            }
            return;
        }

        if(ownerIt != mKeyOwner.end()){
            const Layer* current = _findLayer(ownerIt->second);
            if(current) _dispatchKey(*current, scancode, keycode, keyMod, false);
            mKeyOwner.erase(ownerIt);
            return;
        }

        const std::vector<Layer> layers = mLayers;
        for(const Layer& l : layers){
            if(!_layerAcceptsEvents(l)) continue;
            _dispatchKey(l, scancode, keycode, keyMod, false);
        }
    }

    void InputRouter::injectTextInput(const char* text){
        const std::vector<Layer> layers = mLayers;
        for(const Layer& l : layers){
            if(!_layerAcceptsEvents(l)) continue;
            if(l.kind == BuiltinKind::External){
                if(l.cb.onTextInput && l.cb.onTextInput(text, l.userData)) return;
            }
            else if(l.kind == BuiltinKind::Gui && mGuiInputProcessor){
                mGuiInputProcessor->processTextInput(text);
            }
        }
    }

    void InputRouter::injectTextEdit(const char* text, int selectStart, int selectLength){
        const std::vector<Layer> layers = mLayers;
        for(const Layer& l : layers){
            if(!_layerAcceptsEvents(l)) continue;
            if(l.kind == BuiltinKind::External){
                if(l.cb.onTextEdit && l.cb.onTextEdit(text, selectStart, selectLength, l.userData)) return;
            }
            else if(l.kind == BuiltinKind::Gui && mGuiInputProcessor){
                mGuiInputProcessor->processTextEdit(text, selectStart, selectLength);
            }
        }
    }

    void InputRouter::injectControllerAxis(InputMapper* mapper, InputDeviceId device, int axis, float value, bool isTrigger){
        if(!mInputManager || !mapper) return;

        if(isTrigger){
            const ActionHandle handle = mapper->getAxisMap(device, axis);
            mInputManager->setAnalogTriggerAction(device, handle, value);
            return;
        }

        const bool x = (axis == 0 || axis == 2);
        const ActionHandle handle = mapper->getAxisMap(device, axis);
        const bool hitDeadzone = mInputManager->setAxisAction(device, handle, x, value);
        if(!hitDeadzone && mGuiInputProcessor){
            mGuiInputProcessor->processControllerAxis(*mapper, axis, value, x);
        }
    }

    void InputRouter::injectControllerButton(InputMapper* mapper, InputDeviceId device, int button, bool pressed){
        if(!mInputManager || !mapper) return;

        if(mGuiInputProcessor) mGuiInputProcessor->processControllerButton(*mapper, pressed, button);

        const ActionHandle handle = mapper->getButtonMap(device, button);
        mInputManager->setButtonAction(device, handle, pressed);
    }

    void InputRouter::cancelAllInput(){
        for(int i = 0; i < NUM_ROUTED_MOUSE_BUTTONS; i++){
            if(mMouseButtonOwner[i] == INVALID_INPUT_LAYER) continue;
            const Layer* owner = _findLayer(mMouseButtonOwner[i]);
            if(owner) _dispatchMouseButton(*owner, i, false);
            mMouseButtonOwner[i] = INVALID_INPUT_LAYER;
        }

        for(const auto& entry : mKeyOwner){
            const Layer* owner = _findLayer(entry.second);
            if(owner) _dispatchKey(*owner, entry.first, 0, 0, false);
        }
        mKeyOwner.clear();

        const std::vector<Layer> layers = mLayers;
        for(const Layer& l : layers){
            _notifyCancelled(l);
        }

        if(mPointerFocus != INVALID_INPUT_LAYER){
            const Layer* focused = _findLayer(mPointerFocus);
            if(focused) _notifyPointerFocusLost(*focused);
            mPointerFocus = INVALID_INPUT_LAYER;
        }

        if(mInputManager) mInputManager->releaseAllKeyboardInput();
    }

    bool InputRouter::shouldTextInputEnable() const{
        for(const Layer& l : mLayers){
            if(!_layerAcceptsEvents(l)) continue;
            if(l.kind == BuiltinKind::External){
                if(l.cb.wantsTextInput && l.cb.wantsTextInput(l.userData)) return true;
            }
            else if(l.kind == BuiltinKind::Gui && mGuiInputProcessor){
                if(mGuiInputProcessor->shouldTextInputEnable()) return true;
            }
        }
        return false;
    }

    void InputRouter::getLayerInfo(std::vector<LayerInfo>* outInfo) const{
        outInfo->clear();
        for(const Layer& l : mLayers){
            LayerInfo info;
            info.name = l.name;
            info.priority = l.priority;
            info.enabled = l.enabled;
            info.live = _layerIsLive(l);
            info.external = (l.kind == BuiltinKind::External);
            info.ownsPointer = (mPointerFocus == l.handle);

            info.ownedButtons = 0;
            for(int i = 0; i < NUM_ROUTED_MOUSE_BUTTONS; i++){
                if(mMouseButtonOwner[i] == l.handle) info.ownedButtons++;
            }
            info.ownedKeys = 0;
            for(const auto& entry : mKeyOwner){
                if(entry.second == l.handle) info.ownedKeys++;
            }

            outInfo->push_back(info);
        }
    }
}
