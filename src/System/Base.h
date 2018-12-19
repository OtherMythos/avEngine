#pragma once

#include <memory>

namespace Ogre{
    class Root;
}

namespace AV {
    class SDL2Window;
    
    class Base{
    public:
        Base();
        /**
         Dependency injection constructor. For use only in tests.
         */
        Base(std::shared_ptr<SDL2Window> window);
        ~Base();
        
        /**
         Update the base. This will update all the various subsystems.
         */
        void update();
        
        /**
         @return
         True if the base is running and false if not.
         */
        bool isOpen();
        
        /**
         Shutdown Base and all its subsystems.
         */
        void shutdown();
    
    protected:
        /**
         Initialise Base and all its subsystems.
         */
        void _initialise();
    
    private:
        std::shared_ptr<SDL2Window> _window;
        std::shared_ptr<Ogre::Root> _root;
    
        void _setupOgre();
        void _setupOgreRoot();
        void _registerHLMS();
        
    };
}
