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

        static const char* typeToStr(SQObjectType type) {
            switch (type) {
                case OT_INTEGER: return "INTEGER";
                case OT_FLOAT: return "FLOAT";
                case OT_BOOL: return "BOOL";
                case OT_STRING: return "STRING";
                case OT_TABLE: return "TABLE";
                case OT_ARRAY: return "ARRAY";
                case OT_USERDATA: return "USERDATA";
                case OT_CLOSURE: return "CLOSURE";
                case OT_NATIVECLOSURE: return "NATIVECLOSURE";
                case OT_GENERATOR: return "GENERATOR";
                case OT_USERPOINTER: return "USERPOINTER";
                case OT_THREAD: return "THREAD";
                case OT_FUNCPROTO: return "FUNCPROTO";
                case OT_CLASS: return "CLASS";
                case OT_INSTANCE: return "INSTANCE";
                case OT_WEAKREF: return "WEAKREF";
                case OT_OUTER: return "OUTER";
                default: return "UNKNOWN";
            }
        }

        static void _debugStack(HSQUIRRELVM sq){
            int top = sq_gettop(sq);
            if(top <= 0){
              std::cout << "Nothing in the stack!" << '\n';
              return;
            }
            //This push root table sometimes causes problems.
            //sq_pushroottable(sq);
            while(top >= 0) {
                SQObjectType objectType = sq_gettype(sq, top);
                //Type type = Type(objectType);
                std::cout << "stack index: " << top << " type: " << typeToStr(objectType) << std::endl;
                top--;
            }
        }

    };
}
