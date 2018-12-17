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
    
    void Window::close(){
        _open = false;
    }
    
    bool Window::open(){
        _open = true;
        
        return true;
    }
}
