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
        
        _handleEvents();
        
        SDL_UpdateWindowSurface(_SDLWindow);
    }
    
    bool SDL2Window::open(){
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
    
    void SDL2Window::close(){
        _open = false;
        
        SDL_DestroyWindow(_SDLWindow);
        SDL_Quit();
    }
    
    void SDL2Window::_handleEvents(){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    close();
                    break;
            }
        }
    }
    
}
