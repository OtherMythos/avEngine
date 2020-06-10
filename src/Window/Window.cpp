#include "Window.h"

#include "System/SystemSetup/SystemSettings.h"

namespace AV{
    Window::Window()
    :_width(1600),
    _height(1200),
    _open(false){

    }

    Window::~Window(){

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
}
