#include "Window.h"

#include "System/SystemSetup/SystemSettings.h"

namespace AV{
    Window::Window()
    :_width(1600),
    _height(1200),
    _open(false),
    _minimized(false),
    _fullscreen(false){

    }

    Window::~Window(){

    }

    int Window::getActualWidth() const{
        return _width;
    }

    int Window::getActualHeight() const{
        return _height;
    }

    std::string Window::getDefaultWindowName() const{
        if(!SystemSettings::getWindowTitleSetting().empty()){
            return SystemSettings::getWindowTitleSetting();
        }

        //No title was provided by the user, so return something else.
        if(SystemSettings::getProjectName().empty()){
            return "avEngine";
        }

        return SystemSettings::getProjectName();
    }

    void Window::setTitle(const std::string& title){
        _currentTitle = title;
    }

    void Window::setSystemCursor(SystemCursor cursor){

    }
}
