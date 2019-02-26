#include "SlotManagerNamespace.h"

#include "World/WorldSingleton.h"
#include "World/Slot/SlotManager.h"
#include "Logger/Log.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{

    SQInteger SlotManagerNamespace::setOrigin(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            SlotPosition pos = ScriptUtils::getSlotPositionPopStack(vm);

            world->getSlotManager()->setOrigin(pos);
        }

        return 0;
    }

    SQInteger SlotManagerNamespace::setCurrentMap(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            const SQChar *mapName;
            sq_getstring(vm, -1, &mapName);

            world->getSlotManager()->setCurrentMap(std::string(mapName));
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

    void SlotManagerNamespace::setupNamespace(HSQUIRRELVM vm){
        _addFunction(vm, setOrigin, "setOrigin", 6, ".iinnn");
        _addFunction(vm, setCurrentMap, "setCurrentMap", 2, ".s");
        _addFunction(vm, loadChunk, "loadChunk", 4, ".sii");
        _addFunction(vm, unloadChunk, "unloadChunk", 4, ".sii");
    }
};
