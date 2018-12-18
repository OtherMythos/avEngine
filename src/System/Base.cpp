#include "Base.h"
#include "Logger/Log.h"

#include "Window/SDL2Window/SDL2Window.h"

namespace AV {
    Base::Base(){
        _window = std::make_shared<SDL2Window>();
        
        _initialise();
    }
    
    Base::Base(std::shared_ptr<SDL2Window> window)
    : _window(window){
        
        _initialise();
    }
    
    void Base::_initialise(){
        _window->open();
    }
    
    void Base::update(){
        _window->update();
    }
    
    bool Base::isOpen(){
        return _window->isOpen();
    }
}
