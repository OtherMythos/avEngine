#include "SlotPositionClass.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Vector3UserData.h"

namespace AV{
    SQMemberHandle SlotPositionClass::handleX;
    SQMemberHandle SlotPositionClass::handleY;
    SQMemberHandle SlotPositionClass::handleZ;
    SQMemberHandle SlotPositionClass::handleSlotX;
    SQMemberHandle SlotPositionClass::handleSlotY;

    SQInteger SlotPositionClass::slotPositionAdd(HSQUIRRELVM vm){
        SlotPosition first;
        SlotPosition second;
        bool success = true;
        success &= getSlotFromInstance(vm, -2, &first);
        success &= getSlotFromInstance(vm, -1, &second);

        return slotPositionOperator(vm, first + second);
    }

    SQInteger SlotPositionClass::slotPositionMinus(HSQUIRRELVM vm){
        SlotPosition first;
        SlotPosition second;
        bool success = true;
        success &= getSlotFromInstance(vm, -2, &first);
        success &= getSlotFromInstance(vm, -1, &second);

        return slotPositionOperator(vm, first - second);
    }

    SQInteger SlotPositionClass::slotPositionCompare(HSQUIRRELVM vm){
        SlotPosition first;
        SlotPosition second;
        bool success = true;
        success &= getSlotFromInstance(vm, -2, &first);
        success &= getSlotFromInstance(vm, -1, &second);

        if(first == second){
            sq_pushinteger(vm, 0);
        }else{
            sq_pushinteger(vm, 2);
        }
        return 1;
    }

    SQInteger SlotPositionClass::SlotPositionEquals(HSQUIRRELVM vm){
        SlotPosition first;
        SlotPosition second;
        bool success = true;
        success &= getSlotFromInstance(vm, -2, &first);
        success &= getSlotFromInstance(vm, -1, &second);

        sq_pushbool(vm, first == second);
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
        //TODO -1 should be changed to +1. This is because it's more secure if no value is expected in the stack.
        SlotPosition second;
        bool success = getSlotFromInstance(vm, -1, &second);

        Ogre::Vector3 vec = second.toOgre();

        Vector3UserData::vector3ToUserData(vm, vec);

        return 1;
    }

    SQInteger SlotPositionClass::slotPositionToString(HSQUIRRELVM vm){
        SlotPosition pos;
        bool success = getSlotFromInstance(vm, -1, &pos);

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

        SlotPosition pos;
        bool success = getSlotFromInstance(vm, -1, &pos);

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

    //TODO make this perform type tag checks.
    bool SlotPositionClass::getSlotFromInstance(HSQUIRRELVM vm, SQInteger instanceIndex, SlotPosition* outSlot){
        SQInteger slotX, slotY;
        SQFloat x, y, z;
        slotX = slotY = 0;
        x = y = z = 0;

        bool success = true;
        //TODO this needs sorting out, as it doesn't work properly with negative numbers.
        success &= SQ_SUCCEEDED(sq_getbyhandle(vm, 0 + instanceIndex, &handleZ));
        success &= SQ_SUCCEEDED(sq_getbyhandle(vm, -1 + instanceIndex, &handleY));
        success &= SQ_SUCCEEDED(sq_getbyhandle(vm, -2 + instanceIndex, &handleX));
        success &= SQ_SUCCEEDED(sq_getbyhandle(vm, -3 + instanceIndex, &handleSlotY));
        success &= SQ_SUCCEEDED(sq_getbyhandle(vm, -4 + instanceIndex, &handleSlotX));

        success &= SQ_SUCCEEDED(sq_getinteger(vm, -1, &slotX));
        success &= SQ_SUCCEEDED(sq_getinteger(vm, -2, &slotY));
        success &= SQ_SUCCEEDED(sq_getfloat(vm, -3, &x));
        success &= SQ_SUCCEEDED(sq_getfloat(vm, -4, &y));
        success &= SQ_SUCCEEDED(sq_getfloat(vm, -5, &z));

        if(!success) return false;
        sq_pop(vm, 5);

        SlotPosition slotPos(slotX, slotY, Ogre::Vector3(x, y, z));
        *outSlot = slotPos;
        return true;
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

        ScriptUtils::addFunction(vm, slotPositionConstructor, "constructor");
        ScriptUtils::addFunction(vm, slotPositionAdd, "_add");
        ScriptUtils::addFunction(vm, slotPositionMinus, "_sub");
        ScriptUtils::addFunction(vm, slotPositionToString, "_tostring");
        ScriptUtils::addFunction(vm, slotPositionCompare, "_cmp");
        ScriptUtils::addFunction(vm, toVector3, "toVector3");
        ScriptUtils::addFunction(vm, SlotPositionEquals, "equals");
        ScriptUtils::addFunction(vm, move, "move");


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
