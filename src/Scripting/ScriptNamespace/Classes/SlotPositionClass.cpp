#include "SlotPositionClass.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"
#include "Vector3UserData.h"
#include "Scripting/ScriptObjectTypeTags.h"
#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"

namespace AV{

    SQObject SlotPositionClass::slotPositionDelegateTableObject;

    SQInteger SlotPositionClass::_operatorMetamethod(HSQUIRRELVM vm, OperationType opType){
        SlotPosition first;
        SlotPosition second;
        SCRIPT_ASSERT_RESULT(getSlotFromInstance(vm, -2, &first));
        UserDataGetResult result = getSlotFromInstance(vm, -1, &second);
        if(result != USER_DATA_GET_SUCCESS){
            //It's not a slot position, so check for vector3.
            Ogre::Vector3 outVec;
            result = Vector3UserData::readVector3FromUserData(vm, -1, &outVec);
            if(result == USER_DATA_GET_SUCCESS){
                createNewInstance(vm, opType == OperationType::Add ? first + outVec : first - outVec);
                return 1;
            }else{
                SCRIPT_CHECK_RESULT(result);
            }
        }

        createNewInstance(vm, opType == OperationType::Add ? first + second : first - second);
        return 1;
    }

    SQInteger SlotPositionClass::slotPositionAdd(HSQUIRRELVM vm){
        return _operatorMetamethod(vm, OperationType::Add);
    }

    SQInteger SlotPositionClass::slotPositionMinus(HSQUIRRELVM vm){
        return _operatorMetamethod(vm, OperationType::Subtract);
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
        SCRIPT_ASSERT_RESULT(getSlotFromInstance(vm, -2, &outPos));

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

        SCRIPT_ASSERT_RESULT(_readSlotPositionPtrFromUserData(vm, -2, &first));
        SCRIPT_CHECK_RESULT(_readSlotPositionPtrFromUserData(vm, -1, &second));

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
        SCRIPT_ASSERT_RESULT(_readSlotPositionPtrFromUserData(vm, -2, &first));
        SCRIPT_CHECK_RESULT(_readSlotPositionPtrFromUserData(vm, -1, &second));

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
        SCRIPT_ASSERT_RESULT(_readSlotPositionPtrFromUserData(vm, -1, &pos));

        Ogre::Vector3 vec = pos->toOgre();

        Vector3UserData::vector3ToUserData(vm, vec);

        return 1;
    }

    SQInteger SlotPositionClass::slotPositionToString(HSQUIRRELVM vm){
        SlotPosition* pos;
        SCRIPT_ASSERT_RESULT(_readSlotPositionPtrFromUserData(vm, -1, &pos));

        std::ostringstream stream;
        stream << *pos;
        sq_pushstring(vm, _SC(stream.str().c_str()), -1);

        return 1;
    }

    SQInteger SlotPositionClass::move(HSQUIRRELVM vm){

        Ogre::Vector3 amount;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read3FloatsOrVec3(vm, &amount));

        SlotPosition* pos;
        SCRIPT_ASSERT_RESULT(_readSlotPositionPtrFromUserData(vm, 1, &pos));

        *pos += amount;

        return 0;
    }

    SQInteger SlotPositionClass::moveTowards(HSQUIRRELVM vm){

        SlotPosition* current;
        SCRIPT_ASSERT_RESULT(_readSlotPositionPtrFromUserData(vm, 1, &current));

        SlotPosition* destination;
        SCRIPT_CHECK_RESULT(_readSlotPositionPtrFromUserData(vm, 2, &destination));

        SQFloat amount = 0.0f;
        sq_getfloat(vm, 3, &amount);

        current->moveTowards(*destination, amount);

        return 0;
    }

    UserDataGetResult SlotPositionClass::_readSlotPositionPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, SlotPosition** outPos){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != SlotPositionTypeTag){
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        SlotPosition* foundPos = static_cast<SlotPosition*>(pointer);
        *outPos = foundPos;

        return USER_DATA_GET_SUCCESS;
    }

    UserDataGetResult SlotPositionClass::getSlotFromInstance(HSQUIRRELVM vm, SQInteger stackInx, SlotPosition* outSlot){
        SlotPosition* slotPtr;
        UserDataGetResult result = _readSlotPositionPtrFromUserData(vm, stackInx, &slotPtr);
        if(_readSlotPositionPtrFromUserData(vm, stackInx, &slotPtr) != USER_DATA_GET_SUCCESS) return result;

        *outSlot = *slotPtr;
        return USER_DATA_GET_SUCCESS;
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
        }else if(nargs == 2){
            Ogre::Vector3 vec(Ogre::Vector3::ZERO);
            Vector3UserData::readVector3FromUserData(vm, -1, &vec);
            return SlotPosition(vec);
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
        ScriptUtils::addFunction(vm, moveTowards, "moveTowards", 3, ".un");

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
