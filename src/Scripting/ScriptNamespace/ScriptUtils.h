#pragma once

#include "World/Slot/ChunkCoordinate.h"

namespace AV{
    class ScriptUtils{
    public:
        static ChunkCoordinate getChunkCoordPopStack(HSQUIRRELVM vm){
            SQInteger slotX, slotY;

            sq_getinteger(vm, -1, &slotY);
            sq_getinteger(vm, -2, &slotX);
            const SQChar *mapName;
            sq_getstring(vm, -3, &mapName);

            sq_pop(vm, 3);

            return ChunkCoordinate((int)slotX, (int)slotY, Ogre::String(mapName));
        }

    };
}
