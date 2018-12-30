#pragma once

#include "World.h"

namespace AV {
    
    class WorldSingleton{
    private:
        WorldSingleton() {};
        static World* _world;
        
        
    public:
        WorldSingleton(WorldSingleton const&) = delete;
        void operator=(WorldSingleton const&) = delete;
        
        /**
         Create the world.
         
         @return
         A pointer to the created world.
         If a world already exists, a pointer will be returned to that world, and nothing new will be created.
         */
        static World* createWorld(){
            if(!_world){
                //World hasn't been created.
                _world = new World();
            }
            return _world;
        }
        
        /**
         Destroy the current world.
         
         @remarks
         If there is no current world nothing will be done.
         */
        static void destroyWorld(){
            if(_world != 0){
                delete _world;
                _world = 0;
            }
        }
        
        /**
         Get a pointer to the world.
         
         @return
         A pointer to the world if it exists. 0 if not.
         */
        static World* getWorld(){
            return _world;
        }
        
    };
}
