#include "SlotManagerNamespace.h"

#include "World/WorldSingleton.h"
#include "World/Slot/SlotManager.h"
#include "Classes/SlotPositionClass.h"

namespace AV{

    SQInteger SlotManagerNamespace::setOrigin(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            SlotPosition pos;
            if(!SlotPositionClass::getSlotFromInstance(vm, -1, &pos)) return 0;

            world->getSlotManager()->setOrigin(pos);
        }

        return 0;
    }

    SQInteger SlotManagerNamespace::getOrigin(HSQUIRRELVM vm){
        SlotPosition pos = WorldSingleton::getOrigin();
        SlotPositionClass::instanceFromSlotPosition(vm, pos);

        return 1;
    }

    SQInteger SlotManagerNamespace::setCurrentMap(HSQUIRRELVM vm){
        const SQChar *mapName;
        sq_getstring(vm, -1, &mapName);

        //TODO might have some issues if called during serialisation.
        World *world = WorldSingleton::getWorld();
        if(world){
            world->getSlotManager()->setCurrentMap(std::string(mapName));
        }else{
            //There is currently no world, however we still want to set the map, so that when the world starts up it will read this map and use that.
            WorldSingleton::_setCurrentMapDirect(Ogre::String(mapName));
        }

        return 0;
    }

    SQInteger SlotManagerNamespace::loadChunk(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            ChunkCoordinate coord = ScriptUtils::getChunkCoordPopStack(vm);

            world->getSlotManager()->loadChunk(coord);
        }

        return 0;
    }

    SQInteger SlotManagerNamespace::unloadChunk(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
           ChunkCoordinate coord = ScriptUtils::getChunkCoordPopStack(vm);

            world->getSlotManager()->loadChunk(coord);
        }

        return 0;
    }

    SQInteger SlotManagerNamespace::getCurrentMap(HSQUIRRELVM vm){
        const Ogre::String& s = WorldSingleton::getCurrentMap();

        sq_pushstring(vm, _SC(s.c_str()), -1);

        return 1;
    }

    /**SQNamespace
    @name _slotManager
    @desc Namespace functions relating to the engine's streamable open world.
    */
    void SlotManagerNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name setOrigin
        @desc Set the origin of the world. This will trigger and origin shift.
        @param1:slotPosition:The position to set the origin to.
        */
        ScriptUtils::addFunction(vm, setOrigin, "setOrigin", 2, ".x");
        /**SQFunction
        @name getOrigin
        @desc Obtain a slot position representing the origin of the world.
        */
        ScriptUtils::addFunction(vm, getOrigin, "getOrigin");

        /**SQFunction
        @name setCurrentMap
        @desc Set the current map.
        @param1:mapName:A string representing the target map to switch to.
        */
        ScriptUtils::addFunction(vm, setCurrentMap, "setCurrentMap", 2, ".s");
        /**SQFunction
        @name getCurrentMap
        @returns Gets the current map as a string.
        */
        ScriptUtils::addFunction(vm, getCurrentMap, "getCurrentMap", 1, ".");
        ScriptUtils::addFunction(vm, loadChunk, "loadChunk", 4, ".sii");
        ScriptUtils::addFunction(vm, unloadChunk, "unloadChunk", 4, ".sii");
    }
};
