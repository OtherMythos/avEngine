#include "Base.h"
#include "Logger/Log.h"

#include "Window/SDL2Window/SDL2Window.h"

namespace AV {
    Base::Base(){
        window = std::make_shared<SDL2Window>();
        
        window->open();
    }
    
    void Base::update(){
        AV_INFO("Updating base");
        window->update();
    }
    
    bool Base::isOpen(){
        return window->isOpen();
    }
}
