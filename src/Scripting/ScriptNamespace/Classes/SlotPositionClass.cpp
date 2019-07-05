#include "SlotPositionClass.h"

#include "Logger/Log.h"

namespace AV{
    SQMemberHandle SlotPositionClass::handleX;
    SQMemberHandle SlotPositionClass::handleY;
    SQMemberHandle SlotPositionClass::handleZ;
    SQMemberHandle SlotPositionClass::handleSlotX;
    SQMemberHandle SlotPositionClass::handleSlotY;

    SQInteger SlotPositionClass::slotPositionAdd(HSQUIRRELVM vm){
        SlotPosition first = getSlotFromInstance(vm, -2);
        SlotPosition second = getSlotFromInstance(vm, -1);

        return slotPositionOperator(vm, first + second);
    }

    SQInteger SlotPositionClass::slotPositionMinus(HSQUIRRELVM vm){
        SlotPosition first = getSlotFromInstance(vm, -2);
        SlotPosition second = getSlotFromInstance(vm, -1);

        return slotPositionOperator(vm, first - second);
    }

    SQInteger SlotPositionClass::slotPositionCompare(HSQUIRRELVM vm){
        SlotPosition first = getSlotFromInstance(vm, -2);
        SlotPosition second = getSlotFromInstance(vm, -1);

        if(first == second){
            sq_pushinteger(vm, 0);
        }else{
            sq_pushinteger(vm, 2);
        }
        return 1;
    }

    SQInteger SlotPositionClass::slotPositionOperator(HSQUIRRELVM vm, const SlotPosition& result){
        createNewInstance(vm);

        setInstanceFromSlot(vm, result, 6, -2);

        return 1;
    }

    void SlotPositionClass::instanceFromSlotPosition(HSQUIRRELVM vm, const SlotPosition& pos){
        createNewInstance(vm);

        setInstanceFromSlot(vm, pos, 6, -1);
    }

    void SlotPositionClass::createNewInstance(HSQUIRRELVM vm){
        //OPTIMISATION
        //This might be able to be sped up by keeping a squirrel object reference to the class in the script table.
        //This way you wouldn't have to do a search to find it each time.
        //I need to do some profiling to confirm this.
        sq_pushroottable(vm);
        sq_pushstring(vm, _SC("SlotPosition"), -1);
        sq_get(vm, -2);

        //Create the new instance.
        sq_createinstance(vm, -1);
    }

    SQInteger SlotPositionClass::toVector3(HSQUIRRELVM vm){
        SlotPosition second = getSlotFromInstance(vm, -1);

        Ogre::Vector3 vec = second.toOgre();

        sq_newarray(vm, 3);
        sq_pushfloat(vm, vec.z);
        sq_pushfloat(vm, vec.y);
        sq_pushfloat(vm, vec.x);
        sq_arrayinsert(vm, -4, 0);
        sq_arrayinsert(vm, -3, 1);
        sq_arrayinsert(vm, -2, 2);

        return 1;
    }

    SQInteger SlotPositionClass::slotPositionToString(HSQUIRRELVM vm){
        SlotPosition pos = getSlotFromInstance(vm, -1);

        std::ostringstream stream;
        stream << pos;
        sq_pushstring(vm, _SC(stream.str().c_str()), -1);

        return 1;
    }

    SQInteger SlotPositionClass::move(HSQUIRRELVM vm){
        SQFloat x, y, z;

        sq_getfloat(vm, -1, &z);
        sq_getfloat(vm, -2, &y);
        sq_getfloat(vm, -3, &x);

        sq_pop(vm, 3);

        Ogre::Vector3 ammount(x, y, z);

        SlotPosition pos = getSlotFromInstance(vm, -1);

        pos = pos + ammount;

        setInstanceFromSlot(vm, pos, 6, -1);

        return 0;
    }

    SQInteger SlotPositionClass::slotPositionConstructor(HSQUIRRELVM vm){
        SQInteger nargs = sq_gettop(vm);

        //We need to get it as a slot position to take advantage of the sanity checks the c++ version does.
        SlotPosition pos = getSlotFromStack(vm);

        //pop the provided values to get the instance on the top.
        if(nargs == 6) sq_pop(vm, 5);
        if(nargs == 3) sq_pop(vm, 2);

        setInstanceFromSlot(vm, pos, nargs, -2);

        return 0;
    }

    void SlotPositionClass::setInstanceFromSlot(HSQUIRRELVM vm, const SlotPosition& pos, int argLength, int slotIndex){
        if(argLength == 6){
            sq_pushinteger(vm, pos.chunkX());
            sq_pushinteger(vm, pos.chunkY());
            sq_pushfloat(vm, pos.position().x);
            sq_pushfloat(vm, pos.position().y);
            sq_pushfloat(vm, pos.position().z);

            sq_setbyhandle(vm, -5 + slotIndex, &handleZ);
            sq_setbyhandle(vm, -4 + slotIndex, &handleY);
            sq_setbyhandle(vm, -3 + slotIndex, &handleX);
            sq_setbyhandle(vm, -2 + slotIndex, &handleSlotY);
            sq_setbyhandle(vm, -1 + slotIndex, &handleSlotX);
        }else if(argLength == 3){
            //Just the slot positions.
            sq_pushinteger(vm, pos.chunkX());
            sq_pushinteger(vm, pos.chunkY());
            sq_setbyhandle(vm, -2 + slotIndex, &handleSlotY);
            sq_setbyhandle(vm, -1 + slotIndex, &handleSlotX);
        }
    }

    SlotPosition SlotPositionClass::getSlotFromInstance(HSQUIRRELVM vm, SQInteger instanceIndex){
        SQInteger slotX, slotY;
        SQFloat x, y, z;
        slotX = slotY = 0;
        x = y = z = 0;

        sq_getbyhandle(vm, 0 + instanceIndex, &handleZ);
        sq_getbyhandle(vm, -1 + instanceIndex, &handleY);
        sq_getbyhandle(vm, -2 + instanceIndex, &handleX);
        sq_getbyhandle(vm, -3 + instanceIndex, &handleSlotY);
        sq_getbyhandle(vm, -4 + instanceIndex, &handleSlotX);

        sq_getinteger(vm, -1, &slotX);
        sq_getinteger(vm, -2, &slotY);
        sq_getfloat(vm, -3, &x);
        sq_getfloat(vm, -4, &y);
        sq_getfloat(vm, -5, &z);

        sq_pop(vm, 5);

        return SlotPosition(slotX, slotY, Ogre::Vector3(x, y, z));
    }

    SlotPosition SlotPositionClass::getSlotFromStack(HSQUIRRELVM vm){
        SQInteger nargs = sq_gettop(vm);

        SQInteger slotX, slotY;
        SQFloat x, y, z;
        slotX = slotY = 0;
        x = y = z = 0;

        if(nargs == 6){
            sq_getfloat(vm, -1, &z);
            sq_getfloat(vm, -2, &y);
            sq_getfloat(vm, -3, &x);
            sq_getinteger(vm, -4, &slotY);
            sq_getinteger(vm, -5, &slotX);
        }else if(nargs == 3){
            //Just the slot positions.
            sq_getinteger(vm, -1, &slotY);
            sq_getinteger(vm, -2, &slotX);
        }

        return SlotPosition(slotX, slotY, Ogre::Vector3(x, y, z));
    }

    void SlotPositionClass::setupClass(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("SlotPosition"), -1);
        sq_newclass(vm, 0);

        sq_pushstring(vm, _SC("x"), -1);
        sq_pushfloat(vm, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("y"), -1);
        sq_pushfloat(vm, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("z"), -1);
        sq_pushfloat(vm, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("slotX"), -1);
        sq_pushinteger(vm, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("slotY"), -1);
        sq_pushinteger(vm, 0);
        sq_newslot(vm, -3, false);


        sq_pushstring(vm, _SC("constructor"), -1);
        sq_newclosure(vm, slotPositionConstructor, 0);
        sq_setparamscheck(vm,-1,_SC("xnnn"));
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("_add"), -1);
        sq_newclosure(vm, slotPositionAdd, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("_sub"), -1);
        sq_newclosure(vm, slotPositionMinus, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("_tostring"), -1);
        sq_newclosure(vm, slotPositionToString, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("_cmp"), -1);
        sq_newclosure(vm, slotPositionCompare, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("toVector3"), -1);
        sq_newclosure(vm, toVector3, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("move"), -1);
        sq_newclosure(vm, move, 0);
        sq_newslot(vm, -3, false);


        sq_pushstring(vm, _SC("x"), -1);
        sq_getmemberhandle(vm, -2, &handleX);

        sq_pushstring(vm, _SC("y"), -1);
        sq_getmemberhandle(vm, -2, &handleY);

        sq_pushstring(vm, _SC("z"), -1);
        sq_getmemberhandle(vm, -2, &handleZ);

        sq_pushstring(vm, _SC("slotX"), -1);
        sq_getmemberhandle(vm, -2, &handleSlotX);

        sq_pushstring(vm, _SC("slotY"), -1);
        sq_getmemberhandle(vm, -2, &handleSlotY);


        sq_newslot(vm, -3, false);
    }
}
