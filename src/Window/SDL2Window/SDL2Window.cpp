#include "SDL2Window.h"

#include "Logger/Log.h"
#include "System/SystemSetup/SystemSettings.h"
#include "OgreRenderWindow.h"
#include "OgreStringConverter.h"

#include "Event/Events/SystemEvent.h"
#include "Event/EventDispatcher.h"

#include "Input/Input.h"

#include <iostream>
#include <SDL.h>
#include <SDL_syswm.h>

#ifdef __APPLE__
    #include "MacOS/MacOSUtils.h"
#endif


namespace AV {
    SDL2Window::SDL2Window(){
    SDL_version compiled;
    SDL_version linked;

    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);
    AV_INFO("Built against SDL version {}.{}.{}",
            compiled.major, compiled.minor, compiled.patch);
    AV_INFO("Linking against SDL version {}.{}.{}",
            linked.major, linked.minor, linked.patch);
    }

    SDL2Window::~SDL2Window(){

    }

    void SDL2Window::update(){
        Input::setMouseWheel(0);

        SDL_PumpEvents();

        _pollForEvents();
    }

    bool SDL2Window::open(){
        if(isOpen()){
            //If the window is already open don't open it again.
            return false;
        }

        if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) < 0){
            return false;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
        _SDLWindow = SDL_CreateWindow(SystemSettings::getWindowTitleSetting().c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _width, _height, flags);

        _open = true;
        return true;
    }

    bool SDL2Window::close(){
        if(!isOpen()){
            return false;
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
            case SDL_MOUSEMOTION:
                Input::setMouseX(event.motion.x);
                Input::setMouseY(event.motion.y);
                break;
            case SDL_MOUSEBUTTONDOWN:
                _handleMouseButton((int)event.button.button, true);
                break;
            case SDL_MOUSEBUTTONUP:
                _handleMouseButton((int)event.button.button, false);
                break;
            case SDL_MOUSEWHEEL:
                Input::setMouseWheel(event.wheel.y);
                break;
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

    void SDL2Window::_resizeWindow(SDL_Event &event){
        if(event.window.event != SDL_WINDOWEVENT_RESIZED) return;

        _width = event.window.data1;
        _height = event.window.data2;

        if(_ogreWindow){
            _ogreWindow->resize(_width, _height);
        }
        
        SystemEventWindowResize e;
        e.width = _width;
        e.height = _height;
        
        EventDispatcher::transmitEvent(EventType::System, e);
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
