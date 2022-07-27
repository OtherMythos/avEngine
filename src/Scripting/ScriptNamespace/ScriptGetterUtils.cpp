#include "ScriptGetterUtils.h"

#include "Scripting/ScriptNamespace/Classes/Vector2UserData.h"
#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "Scripting/ScriptNamespace/Classes/ColourValueUserData.h"
#include "Scripting/ScriptNamespace/Classes/SlotPositionClass.h"

namespace AV{
    SQInteger ScriptGetterUtils::vector3ReadSlotOrVec(HSQUIRRELVM vm, Ogre::Vector3* outVec, SQInteger idx){
        if(Vector3UserData::readVector3FromUserData(vm, idx, outVec) == USER_DATA_GET_SUCCESS) return 0;

        SlotPosition pos;
        bool success = SlotPositionClass::getSlotFromInstance(vm, idx, &pos) == USER_DATA_GET_SUCCESS;
        if(success){
            *outVec = pos.toOgre();
            return 0;
        }
        return sq_throwerror(vm, "Expected either a Vec3 or SlotPosition.");
    }

    SQInteger ScriptGetterUtils::vector3Read(HSQUIRRELVM vm, Ogre::Vector3* outVec){
        SQInteger size = sq_gettop(vm);

        if(size == 2){
            //Vector3

            return vector3ReadSlotOrVec(vm, outVec, -1);
        }else if(size == 4){
            //Regular

            bool success = true;
            SQFloat x, y, z;
            success &= SQ_SUCCEEDED(sq_getfloat(vm, -1, &z));
            success &= SQ_SUCCEEDED(sq_getfloat(vm, -2, &y));
            success &= SQ_SUCCEEDED(sq_getfloat(vm, -3, &x));
            if(!success) return sq_throwerror(vm, "Error reading provided coordinates.");

            *outVec = Ogre::Vector3(x, y, z);

            sq_pop(vm, 3);
        }else return sq_throwerror(vm, "Error parsing coordinates.");

        return 0;
    }

    UserDataGetResult ScriptGetterUtils::read4FloatsOrColourValue(HSQUIRRELVM vm, Ogre::ColourValue* outCol){
        SQInteger size = sq_gettop(vm);

        if(size == 2){

            ColourValueUserData::readColourValueFromUserData(vm, 2, outCol);
        }else if(size == 5){
            //Regular

            bool success = true;
            SQFloat r, g, b, a;
            success &= SQ_SUCCEEDED(sq_getfloat(vm, -1, &a));
            success &= SQ_SUCCEEDED(sq_getfloat(vm, -2, &b));
            success &= SQ_SUCCEEDED(sq_getfloat(vm, -3, &g));
            success &= SQ_SUCCEEDED(sq_getfloat(vm, -4, &r));
            if(!success) return USER_DATA_GET_INCORRECT_TYPE;

            *outCol = Ogre::ColourValue(r, g, b, a);

            sq_pop(vm, 4);
        }else return USER_DATA_ERROR;

        return USER_DATA_GET_SUCCESS;
    }

    UserDataGetResult ScriptGetterUtils::read3FloatsOrVec3(HSQUIRRELVM vm, Ogre::Vector3* outVec){
        SQInteger size = sq_gettop(vm);

        if(size == 2){
            //Vector3

            return Vector3UserData::readVector3FromUserData(vm, -1, outVec);
        }else if(size == 4){
            //Regular

            bool success = true;
            SQFloat x, y, z;
            success &= SQ_SUCCEEDED(sq_getfloat(vm, -1, &z));
            success &= SQ_SUCCEEDED(sq_getfloat(vm, -2, &y));
            success &= SQ_SUCCEEDED(sq_getfloat(vm, -3, &x));
            if(!success) return USER_DATA_GET_INCORRECT_TYPE;

            *outVec = Ogre::Vector3(x, y, z);

            sq_pop(vm, 3);
        }else return USER_DATA_ERROR;

        return USER_DATA_GET_SUCCESS;
    }

    UserDataGetResult ScriptGetterUtils::read2FloatsOrVec2(HSQUIRRELVM vm, Ogre::Vector2* outVec){
        SQInteger size = sq_gettop(vm);

        if(size == 2){
            //Vector2

            return Vector2UserData::readVector2FromUserData(vm, -1, outVec);
        }else if(size == 3){
            //Regular

            bool success = true;
            SQFloat x, y;
            success &= SQ_SUCCEEDED(sq_getfloat(vm, -1, &y));
            success &= SQ_SUCCEEDED(sq_getfloat(vm, -2, &x));
            if(!success) return USER_DATA_GET_INCORRECT_TYPE;

            *outVec = Ogre::Vector2(x, y);

            sq_pop(vm, 2);
        }else return USER_DATA_ERROR;

        return USER_DATA_GET_SUCCESS;
    }
}
