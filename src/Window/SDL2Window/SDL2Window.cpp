#include "SDL2Window.h"

#include "Logger/Log.h"
#include "System/SystemSetup/SystemSettings.h"
#include "OgreRenderWindow.h"
#include "OgreStringConverter.h"

#include "Event/Events/SystemEvent.h"
#include "Event/EventDispatcher.h"

#include <OgreWindowEventUtilities.h>

#include "Input/Input.h"

#include <iostream>
#include <SDL.h>
#include <SDL_syswm.h>

#include "Input/InputManager.h"

#ifdef __APPLE__
    #include "MacOS/MacOSUtils.h"
#endif


namespace AV {
    SDL2Window::SDL2Window(){
        SDL_version compiled;
        SDL_version linked;

        SDL_VERSION(&compiled);
        SDL_GetVersion(&linked);
        AV_INFO("Built against SDL version {}.{}.{}", compiled.major, compiled.minor, compiled.patch);
        AV_INFO("Linking against SDL version {}.{}.{}", linked.major, linked.minor, linked.patch);

        for(int i = 0; i < MAX_INPUT_DEVICES; i++){
            mOpenGameControllers[i] = {0, 0};
        }
    }

    SDL2Window::~SDL2Window(){

    }

    void SDL2Window::update(){
        Input::setMouseWheel(0);

        Ogre::WindowEventUtilities::messagePump();
        SDL_PumpEvents();

        _pollForEvents();
    }

    bool SDL2Window::open(InputManager* inputMan){
        if(isOpen()){
            //If the window is already open don't open it again.
            return false;
        }

        _width = SystemSettings::getDefaultWindowWidth();
        _height = SystemSettings::getDefaultWindowHeight();

        if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) < 0){
            return false;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
        if(SystemSettings::isWindowResizable()){
            flags |= SDL_WINDOW_RESIZABLE;
        }
        _SDLWindow = SDL_CreateWindow(SystemSettings::getWindowTitleSetting().c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _width, _height, flags);

        _open = true;
        inputMapper.initialise(inputMan);
        inputMapper.setupMap();
        mInputManager = inputMan;

        return true;
    }

    bool SDL2Window::close(){
        if(!isOpen()){
            return false;
        }

        for(int i = 0; i < MAX_INPUT_DEVICES; i++){
            if(!mOpenGameControllers[i].controller) continue;
            SDL_GameControllerClose(mOpenGameControllers[i].controller);
        }

        _open = false;
        _ogreWindow->destroy();

        SDL_DestroyWindow(_SDLWindow);
        SDL_Quit();

        return true;
    }

    void SDL2Window::_pollForEvents(){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            _handleEvent(event);
        }
    }

    void SDL2Window::_handleEvent(SDL_Event &event){
        switch(event.type){
            case SDL_QUIT:
                //close();
                wantsToClose = true;
                break;
            case SDL_WINDOWEVENT:
                switch(event.window.event){
	                case SDL_WINDOWEVENT_RESIZED:
	                    _resizeWindow(event);
	                    break;
	            }
            case SDL_KEYDOWN:
                if(event.key.repeat == 0)
                    this->_handleKey(event.key.keysym, true);
                break;
            case SDL_KEYUP:
                if(event.key.repeat == 0)
                    _handleKey(event.key.keysym, false);
                break;
            case SDL_MOUSEMOTION:{
                int w, h;
                SDL_GL_GetDrawableSize(_SDLWindow, &w, &h);
                float actualWidth = w / _width;

                Input::setMouseX(event.motion.x * actualWidth);
                Input::setMouseY(event.motion.y * actualWidth);
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
                _handleMouseButton((int)event.button.button, true);
                break;
            case SDL_MOUSEBUTTONUP:
                _handleMouseButton((int)event.button.button, false);
                break;
            case SDL_MOUSEWHEEL:
                Input::setMouseWheel(event.wheel.y);
                break;
            case SDL_CONTROLLERAXISMOTION:{
                _handleControllerAxis(event);
                break;
            }
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:{
                _handleControllerButton(event);
                break;
            }
            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:{
                _handleDeviceChange(event);
                break;
            }
        }
    }

    void SDL2Window::injectOgreWindow(Ogre::RenderWindow *window){
        _ogreWindow = window;
        window->resize(_width, _height);
    }

    Ogre::String SDL2Window::getHandle(){
        SDL_SysWMinfo wmInfo;
        SDL_VERSION( &wmInfo.version );

        if(!SDL_GetWindowWMInfo(_SDLWindow, &wmInfo)){
            AV_CRITICAL("SDL failed to query window information to obtain the window handle: {}", SDL_GetError());
            //TODO Get it to stop execution here
            return "0";
        }

        #ifdef __APPLE__
            return Ogre::StringConverter::toString(WindowContentViewHandle(wmInfo));
        #elif __linux__
            return Ogre::StringConverter::toString( (uintptr_t) wmInfo.info.x11.window);
		#elif _WIN32
			return Ogre::StringConverter::toString((uintptr_t)wmInfo.info.win.window);
        #endif
    }

    void SDL2Window::grabCursor(bool capture){
        SDL_SetRelativeMouseMode(capture ? SDL_TRUE : SDL_FALSE);

        //int type = capture ? SDL_DISABLE : SDL_ENABLE;
        //SDL_ShowCursor(type);
    }

    void SDL2Window::_resizeWindow(SDL_Event &event){
        if(event.window.event != SDL_WINDOWEVENT_RESIZED) return;

        _width = event.window.data1;
        _height = event.window.data2;

        if(_ogreWindow){
            #ifdef _WIN32
                _ogreWindow->windowMovedOrResized();
            #else
                _ogreWindow->resize(_width, _height);
            #endif
        }

        SystemEventWindowResize e;
        e.width = _width;
        e.height = _height;

        EventDispatcher::transmitEvent(EventType::System, e);
    }

    void SDL2Window::_handleControllerAxis(const SDL_Event& e){
        assert(e.type == SDL_CONTROLLERAXISMOTION);

        bool isTrigger = e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT;

        InputDeviceId deviceId = (InputDeviceId)e.cbutton.which;
        ActionHandle handle = inputMapper.getAxisMap(deviceId, (int)e.caxis.axis);
        //32767 is the maximum number sdl will return for an axis.
        float normValue = float(e.caxis.value) / 32767.0f;
        if(isTrigger){
            mInputManager->setAnalogTriggerAction(deviceId, handle, normValue);
        }else{
            //It's an actual axis.
            bool x = (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX || e.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX) ? true : false;
            mInputManager->setAxisAction(deviceId, handle, x, normValue);
        }
    }

    void SDL2Window::_handleControllerButton(const SDL_Event& e){
        assert(e.type == SDL_CONTROLLERBUTTONDOWN || e.type == SDL_CONTROLLERBUTTONUP);

        InputDeviceId deviceId = (InputDeviceId)e.cbutton.which;
        ActionHandle handle = inputMapper.getButtonMap(deviceId, (int)e.cbutton.button);
        mInputManager->setButtonAction(deviceId, handle, e.cbutton.state == SDL_PRESSED ? true : false);
    }

    void SDL2Window::_handleDeviceChange(const SDL_Event& e){
        assert(e.type == SDL_CONTROLLERDEVICEADDED || e.type == SDL_CONTROLLERDEVICEREMOVED);

        InputDeviceId devId = e.cdevice.which;
        if(devId >= MAX_INPUT_DEVICES) return;

        if(e.type == SDL_CONTROLLERDEVICEADDED){
            const char* devName = SDL_GameControllerNameForIndex(e.cdevice.which);
            InputDeviceId id = mInputManager->addInputDevice(devName);
            //If we've run out of available controllers, just don't add this new one.
            if(id == INVALID_INPUT_DEVICE) return;

            SDL_GameController* addDevice = SDL_GameControllerOpen(e.cdevice.which);
            assert(addDevice);

            SDL_Joystick* j = SDL_GameControllerGetJoystick(addDevice);
            SDL_JoystickID joyId = SDL_JoystickInstanceID(j);

            //TODO destroy the controller if this fails.
            //const char* devName = SDL_GameControllerName(addDevice);

            mOpenGameControllers[devId] = {addDevice, id};
        }else{
            SDL_GameController* controller = SDL_GameControllerFromInstanceID(devId);
            assert(controller);

            int targetIdx = -1;
            //Find the id based on the pointer.
            for(int i = 0; i < MAX_INPUT_DEVICES; i++){
                if(controller == mOpenGameControllers[i].controller){
                    targetIdx = i;
                    break;
                }
            }
            assert(targetIdx >= 0); //It should be somewhere in the list.

            mInputManager->removeInputDevice(mOpenGameControllers[targetIdx].controllerId);

            SDL_GameControllerClose(controller);
            mOpenGameControllers[targetIdx] = {0, 0};
        }
    }

    void SDL2Window::_handleKey(SDL_Keysym key, bool pressed){
        auto k = key.sym;
        Input::Input_Key retKey = Input::Key_Null;

        if(k == SDLK_w) retKey = Input::Key_Up;
        else if(k == SDLK_s) retKey = Input::Key_Down;
        else if(k == SDLK_a) retKey = Input::Key_Left;
        else if(k == SDLK_d) retKey = Input::Key_Right;

        else if(k == SDLK_h) retKey = Input::Key_Accept;
        else if(k == SDLK_g) retKey = Input::Key_Decline;

        else if(k == SDLK_F5) retKey = Input::Key_DeveloperGuiToggle;

        Input::setKeyActive(retKey, pressed);

        //New stuff. The previous will be deleted soon.
        ActionHandle handle = inputMapper.getKeyboardMap((int)key.sym);
        //Right now pass as item 0. In future the keyboard might get its own device.
        mInputManager->setButtonAction(0, handle, pressed);
    }

    void SDL2Window::_handleMouseButton(int button, bool pressed){
        int targetButton = 0;
        switch(button){
            case SDL_BUTTON_LEFT:
                targetButton = 0;
                break;
            case SDL_BUTTON_RIGHT:
                targetButton = 1;
                break;
            case SDL_BUTTON_MIDDLE:
                targetButton = 2;
                break;
        }

        Input::setMouseButton(targetButton, pressed);
    }

    bool SDL2Window::isOpen(){
        return _open;
    }

}
