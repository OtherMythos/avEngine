#include "SlotManagerNamespace.h"

#include "World/WorldSingleton.h"
#include "World/Slot/SlotManager.h"
#include "Logger/Log.h"

namespace AV{
    ChunkCoordinate SlotManagerNamespace::_getChunkCoordPopStack(HSQUIRRELVM vm){
        SQInteger slotX, slotY;

        sq_getinteger(vm, -1, &slotY);
        sq_getinteger(vm, -2, &slotX);
        const SQChar *mapName;
        sq_getstring(vm, -3, &mapName);

        sq_pop(vm, 3);

        return ChunkCoordinate((int)slotX, (int)slotY, Ogre::String(mapName));
    }

    SQInteger SlotManagerNamespace::setOrigin(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            SQInteger slotX, slotY;
            SQFloat x, y, z;
            sq_getfloat(vm, -1, &z);
            sq_getfloat(vm, -2, &y);
            sq_getfloat(vm, -3, &x);

            sq_getinteger(vm, -4, &slotY);
            sq_getinteger(vm, -5, &slotX);

            world->getSlotManager()->setOrigin(SlotPosition(slotX, slotY, Ogre::Vector3(x, y, z)));
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
            ChunkCoordinate coord = _getChunkCoordPopStack(vm);

            world->getSlotManager()->loadChunk(coord);
        }

        return 0;
    }

    SQInteger SlotManagerNamespace::unloadChunk(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
           ChunkCoordinate coord = _getChunkCoordPopStack(vm);

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
