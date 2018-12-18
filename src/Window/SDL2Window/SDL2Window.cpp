#include "SDL2Window.h"

#include <iostream>
#include <SDL_syswm.h>
#include <SDL.h>

#include "Logger/Log.h"

namespace AV {
    SDL2Window::SDL2Window(){
        SDL_version compiled;
        SDL_version linked;
        
        SDL_VERSION(&compiled);
        SDL_GetVersion(&linked);
        AV_INFO("Build against SDL version {}.{}.{}",
                compiled.major, compiled.minor, compiled.patch);
        AV_INFO("Linking against SDL version {}.{}.{}",
                linked.major, linked.minor, linked.patch);
    }
    
    SDL2Window::~SDL2Window(){
        
    }
    
    void SDL2Window::update(){
        SDL_PumpEvents();
        
        _pollForEvents();
        
        SDL_UpdateWindowSurface(_SDLWindow);
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
        _SDLWindow = SDL_CreateWindow("Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _width, _height, flags);
        
        _open = true;
        return true;
    }
    
    bool SDL2Window::close(){
        if(!isOpen()){
            return false;
        }
        
        _open = false;
        
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
                close();
                break;
            case SDL_KEYDOWN:
                if(event.key.repeat == 0)
                    this->_handleKey(event.key.keysym, true);
                break;
            case SDL_KEYUP:
                if(event.key.repeat == 0)
                    _handleKey(event.key.keysym, false);
                break;
        }
    }
    
    void SDL2Window::_handleKey(SDL_Keysym key, bool pressed){
        AV_INFO("Hello")
        //AV_INFO("{}", key.sym);
    }
    
    bool SDL2Window::isOpen(){
        return _open;
    }
    
}
