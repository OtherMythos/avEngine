#pragma once

#include "World.h"
#include "Slot/SlotPosition.h"

namespace AV {
    class SlotManager;
    class SaveHandle;
    class World;

    /**
     A class to manage the creation and destruction of the world.
     */
    class WorldSingleton{
        //The Slot Manager needs to be able to set the origin.
        friend SlotManager;
        friend World;
    private:
        WorldSingleton() {};
        static World* _world;
        static bool mWorldReady;

        static SlotPosition _origin;
        static SlotPosition _playerPosition;
        static Ogre::String mCurrentMap;
        static int mPlayerLoadRadius;


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
                //Creating world without save, so it will be immediately ready.
                mWorldReady = true;
                return true;
            }
            return false;
        }

        /**
        Create the world from a save.

        @remarks
        With this creation method, the world will not be immediately ready.
        This is because loading from a save is a threaded operation.
        The world will become ready as soon as this operation completes.

        @return
        True or false as to whether or not a new world was created, i.e if there is already a world this function will return false.
        */
        static bool createWorld(const SaveHandle &handle){
            if(!_world){
                _world = new World(handle);
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
         If the current world is not ready then nothing will be done.
         This is because the deserialisation is a threaded operation, and the world cannot be destroyed until all threads are finished.
         If you need to destroy a non-ready world, wait until it's finished loading and becomes ready.
         */
        static bool destroyWorld(){
            //The world has to be ready before deletion can take place.
            if(_world != 0 && mWorldReady){
                delete _world;
                _world = 0;
                mWorldReady = false;
                return true;
            }
            return false;
        }

        /**
         Get a pointer to the world. This will not return a pointer if the world is not ready.

         @return
         A pointer to the world if it exists and is ready. 0 if not.
         */
        static World* getWorld(){
            if(!mWorldReady) return 0;
            return _world;
        }

        /**
         Get a pointer to the world, without doing any ready checks.
         Most functions will want to use getWorld() rather than this.

         @return
         A pointer to the world if it exists. 0 if not.
         */
        static World* getWorldNoCheck(){
            return _world;
        }

        /**
        Determine whether the world is ready or not.
        Ready means whether the world has been created AND all relevant save data has been parsed and loaded.
        Save data is loaded by worker threads, meaning that there might be some time between the world being created and being 'ready'.

        @remarks
        If the world isn't ready, nothing should be calling its functions, as it should be in a read only state.
        */
        static bool worldReady(){ return mWorldReady; }

        static const SlotPosition& getOrigin(){
            return _origin;
        }

        static const SlotPosition& getPlayerPosition(){
            return _playerPosition;
        }

        static const Ogre::String& getCurrentMap(){
            return mCurrentMap;
        }


        static int getPlayerLoadRadius(){
            return mPlayerLoadRadius;
        }

        static void setPlayerPosition(const SlotPosition& pos);

        static void setPlayerLoadRadius(int radius);

    };
}
