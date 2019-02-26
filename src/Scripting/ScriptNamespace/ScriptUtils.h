#pragma once

#include "World/Slot/ChunkCoordinate.h"
#include "World/Slot/SlotPosition.h"

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
        
        static SlotPosition getSlotPositionPopStack(HSQUIRRELVM vm){
            SQInteger slotX, slotY;
            SQFloat x, y, z;
            sq_getfloat(vm, -1, &z);
            sq_getfloat(vm, -2, &y);
            sq_getfloat(vm, -3, &x);
            
            sq_getinteger(vm, -4, &slotY);
            sq_getinteger(vm, -5, &slotX);
            
            sq_pop(vm, 5);
            
            return SlotPosition(slotX, slotY, Ogre::Vector3(x, y, z));
        }

    };
}
