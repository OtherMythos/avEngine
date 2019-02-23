#pragma once

#include "World.h"
#include "Slot/SlotPosition.h"

namespace AV {
    class SlotManager;

    /**
     A class to manage the creation and destruction of the world.
     */
    class WorldSingleton{
        //The Slot Manager needs to be able to set the origin.
        friend SlotManager;
    private:
        WorldSingleton() {};
        static World* _world;

        static SlotPosition _origin;
        static SlotPosition _playerPosition;
        static Ogre::String mCurrentMap;
        static int playerLoadRadius;


    public:
        WorldSingleton(WorldSingleton const&) = delete;
        void operator=(WorldSingleton const&) = delete;

        /**
         Create the world.

         @return
         True or false as to whether or not a new world was created, i.e if there is already a world this function will return false.
         */
        static bool createWorld(){
            if(!_world){
                //World hasn't been created.
                _world = new World();
                return true;
            }
            return false;
        }

        /**
         Destroy the current world.

         @return
         True or false as to whether or not the world was destroyed, i.e if there is no world nothing will be destroyed, and false will be returned.

         @remarks
         If there is no current world nothing will be done.
         */
        static bool destroyWorld(){
            if(_world != 0){
                delete _world;
                _world = 0;
                return true;
            }
            return false;
        }

        /**
         Get a pointer to the world.

         @return
         A pointer to the world if it exists. 0 if not.
         */
        static World* getWorld(){
            return _world;
        }

        static const SlotPosition& getOrigin(){
            return _origin;
        }

        static const SlotPosition& getPlayerPosition(){
            return _playerPosition;
        }

        static const Ogre::String& getCurrentMap(){
            return mCurrentMap;
        }

        static void setPlayerPosition(const SlotPosition& pos){
            _playerPosition = pos;
        }

        static int getPlayerLoadRadius(){
            return playerLoadRadius;
        }

        static void setPlayerLoadRadius(int radius){
            playerLoadRadius = radius;
        }

    };
}
