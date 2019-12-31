#include "SlotManagerNamespace.h"

#include "World/WorldSingleton.h"
#include "World/Slot/SlotManager.h"
#include "Classes/SlotPositionClass.h"

namespace AV{

    SQInteger SlotManagerNamespace::setOrigin(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            SlotPosition pos = SlotPositionClass::getSlotFromInstance(vm, -1);

            world->getSlotManager()->setOrigin(pos);
        }

        return 0;
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
        World *world = WorldSingleton::getWorld();
        if(world){
            ChunkCoordinate coord = ScriptUtils::getChunkCoordPopStack(vm);

            world->getSlotManager()->loadChunk(coord);
        }

        return 0;
    }

    SQInteger SlotManagerNamespace::unloadChunk(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
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

    void SlotManagerNamespace::setupNamespace(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, setOrigin, "setOrigin", 2, ".x");
        ScriptUtils::addFunction(vm, setCurrentMap, "setCurrentMap", 2, ".s");
        ScriptUtils::addFunction(vm, getCurrentMap, "getCurrentMap", 1, ".");
        ScriptUtils::addFunction(vm, loadChunk, "loadChunk", 4, ".sii");
        ScriptUtils::addFunction(vm, unloadChunk, "unloadChunk", 4, ".sii");
    }
};
