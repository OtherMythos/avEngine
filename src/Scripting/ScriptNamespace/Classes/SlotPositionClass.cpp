#include "SlotPositionClass.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"
#include "Vector3UserData.h"
#include "Scripting/ScriptObjectTypeTags.h"

namespace AV{

    SQObject SlotPositionClass::slotPositionDelegateTableObject;

    SQInteger SlotPositionClass::slotPositionAdd(HSQUIRRELVM vm){
        SlotPosition first;
        SlotPosition second;
        bool success = true;
        success &= getSlotFromInstance(vm, -2, &first);
        success &= getSlotFromInstance(vm, -1, &second);

        createNewInstance(vm, first + second);
        return 1;
    }

    SQInteger SlotPositionClass::slotPositionMinus(HSQUIRRELVM vm){
        SlotPosition first;
        SlotPosition second;
        bool success = true;
        success &= getSlotFromInstance(vm, -2, &first);
        success &= getSlotFromInstance(vm, -1, &second);

        createNewInstance(vm, first - second);
        return 1;
    }

    SQInteger SlotPositionClass::SlotPositionGet(HSQUIRRELVM vm){
        enum class TargetType{
            X, Y, Z, SlotX, SlotY, None
        };

        const SQChar *key;
        sq_getstring(vm, -1, &key);

        TargetType foundType = TargetType::None;
        if(strcmp(key, "x") == 0) foundType = TargetType::X;
        else if(strcmp(key, "y") == 0) foundType = TargetType::Y;
        else if(strcmp(key, "z") == 0) foundType = TargetType::Z;
        else if(strcmp(key, "slotX") == 0) foundType = TargetType::SlotX;
        else if(strcmp(key, "slotY") == 0) foundType = TargetType::SlotY;

        if(foundType == TargetType::None){
            sq_pushnull(vm);
            return sq_throwobject(vm);
        }

        SlotPosition outPos;
        if(!getSlotFromInstance(vm, -2, &outPos)){
            //Should not happen.
            assert(false);
            return 0;
        }

        switch(foundType){
            case TargetType::X: sq_pushfloat(vm, outPos.position().x); break;
            case TargetType::Y: sq_pushfloat(vm, outPos.position().y); break;
            case TargetType::Z: sq_pushfloat(vm, outPos.position().z); break;
            case TargetType::SlotX: sq_pushinteger(vm, outPos.chunkX()); break;
            case TargetType::SlotY: sq_pushinteger(vm, outPos.chunkY()); break;
            default:
                assert(false);
                break;
        }

        return 1;
    }

    SQInteger SlotPositionClass::slotPositionCompare(HSQUIRRELVM vm){
        SlotPosition* first;
        SlotPosition* second;
        bool success = true;
        success &= _readSlotPositionPtrFromUserData(vm, -2, &first);
        success &= _readSlotPositionPtrFromUserData(vm, -1, &second);

        if(*first == *second){
            sq_pushinteger(vm, 0);
        }else{
            sq_pushinteger(vm, 2);
        }
        return 1;
    }

    SQInteger SlotPositionClass::SlotPositionEquals(HSQUIRRELVM vm){
        SlotPosition* first;
        SlotPosition* second;
        bool success = true;
        success &= _readSlotPositionPtrFromUserData(vm, -2, &first);
        success &= _readSlotPositionPtrFromUserData(vm, -1, &second);

        sq_pushbool(vm, *first == *second);
        return 1;
    }

    void SlotPositionClass::createNewInstance(HSQUIRRELVM vm, const SlotPosition& pos){
        SlotPosition* pointer = (SlotPosition*)sq_newuserdata(vm, sizeof(SlotPosition));
        *pointer = pos;

        sq_pushobject(vm, slotPositionDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, SlotPositionTypeTag);
    }

    SQInteger SlotPositionClass::toVector3(HSQUIRRELVM vm){
        SlotPosition* pos;
        bool success = _readSlotPositionPtrFromUserData(vm, -1, &pos);

        Ogre::Vector3 vec = pos->toOgre();

        Vector3UserData::vector3ToUserData(vm, vec);

        return 1;
    }

    SQInteger SlotPositionClass::slotPositionToString(HSQUIRRELVM vm){
        SlotPosition* pos;
        bool success = _readSlotPositionPtrFromUserData(vm, -1, &pos);

        std::ostringstream stream;
        stream << *pos;
        sq_pushstring(vm, _SC(stream.str().c_str()), -1);

        return 1;
    }

    SQInteger SlotPositionClass::move(HSQUIRRELVM vm){

        Ogre::Vector3 amount;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read3FloatsOrVec3(vm, &amount));

        SlotPosition* pos;
        bool success = _readSlotPositionPtrFromUserData(vm, -1, &pos);

        *pos += amount;

        return 0;
    }

    bool SlotPositionClass::_readSlotPositionPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, SlotPosition** outPos){
        //TODO update to return the error types rather than this.
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return false;
        if(typeTag != SlotPositionTypeTag){
            //*outSlot = 0;
            return false;
        }

        SlotPosition* foundPos = static_cast<SlotPosition*>(pointer);
        *outPos = foundPos;

        return true;
    }

    bool SlotPositionClass::getSlotFromInstance(HSQUIRRELVM vm, SQInteger stackInx, SlotPosition* outSlot){
        SlotPosition* slotPtr;
        if(!_readSlotPositionPtrFromUserData(vm, stackInx, &slotPtr)) return false;

        *outSlot = *slotPtr;
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

    SQInteger SlotPositionClass::createSlotPosition(HSQUIRRELVM vm){
        SlotPosition pos = getSlotFromStack(vm);

        createNewInstance(vm, pos);
        return 1;
    }

    void SlotPositionClass::setupClass(HSQUIRRELVM vm){
        sq_newtableex(vm, 11);

        ScriptUtils::addFunction(vm, slotPositionAdd, "_add", 2, "uu");
        ScriptUtils::addFunction(vm, slotPositionMinus, "_sub", 2, "uu");
        ScriptUtils::addFunction(vm, slotPositionToString, "_tostring");
        ScriptUtils::addFunction(vm, slotPositionCompare, "_cmp", 2, "uu");
        ScriptUtils::addFunction(vm, SlotPositionGet, "_get");
        ScriptUtils::addFunction(vm, toVector3, "toVector3");
        ScriptUtils::addFunction(vm, SlotPositionEquals, "equals", 2, "uu");
        ScriptUtils::addFunction(vm, move, "move", -2, ".n|unn");

        sq_resetobject(&slotPositionDelegateTableObject);
        sq_getstackobj(vm, -1, &slotPositionDelegateTableObject);
        sq_addref(vm, &slotPositionDelegateTableObject);
        sq_pop(vm, 1);

        {
            sq_pushroottable(vm);
            //TODO proper type checks in here.
            ScriptUtils::addFunction(vm, createSlotPosition, "SlotPosition");
            sq_pop(vm, 1);
        }

    }
}
