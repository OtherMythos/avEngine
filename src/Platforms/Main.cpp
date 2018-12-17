#include "Window/SDL2Window/SDL2Window.h"
#include "Logger/Log.h"

int main(){
    AV::Log::Init();
    
    AV_INFO("Starting");
    
    AV::Window* window = new AV::SDL2Window();
    window->open();

    while(window->isOpen()){
        window->update();
    }

    window->close();
    delete window;

    return 0;
}
