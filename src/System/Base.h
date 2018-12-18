#pragma once

#include <memory>

namespace AV {
    class SDL2Window;
    
    class Base{
    public:
        Base();
        Base(std::shared_ptr<SDL2Window> window);
        
        /**
         Update the base. This will update all the various subsystems.
         */
        void update();
        
        /**
         @return
         True if the base is running and false if not.
         */
        bool isOpen();
    
    protected:
        void _initialise();
    
    private:
        std::shared_ptr<SDL2Window> _window;
    };
}
