#include "Window.h"

namespace AV{
	Window::Window()
    :_width(800),
    _height(600),
    _open(false){
        
    }
    
    Window::~Window(){
        
    }
    
    void Window::update(){
        
    }
    
    bool Window::close(){
        if(!_open) return false;
        
        _open = false;
        
        return true;
    }
    
    bool Window::open(){
        _open = true;
        
        return true;
    }
}
