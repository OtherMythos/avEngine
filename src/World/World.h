#pragma once

#include "Logger/Log.h"

namespace AV {
    class WorldSingleton;
    
    class World{
        friend WorldSingleton;
    protected:
        World();
        ~World();
        
    public:
        void doSomething(){
            AV_INFO("World");
        }
    };
}
