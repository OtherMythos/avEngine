#include "Vector3UserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

namespace AV{

    SQObject Vector3UserData::vector3DelegateTableObject;

    void Vector3UserData::setupTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 1);

        ScriptUtils::addFunction(vm, setMetamethod, "_set");


        sq_resetobject(&vector3DelegateTableObject);
        sq_getstackobj(vm, -1, &vector3DelegateTableObject);
        sq_addref(vm, &vector3DelegateTableObject);
        sq_pop(vm, 1);

        //Create the creation functions.
        sq_pushroottable(vm);

        {
            ScriptUtils::addFunction(vm, createVector3, "Vec3", -4, ".nnn");
        }

        sq_pop(vm, 1);
    }

    SQInteger Vector3UserData::createVector3(HSQUIRRELVM vm){
        vector3ToUserData(vm, Ogre::Vector3::ZERO);

        return 1;
    }

    SQInteger Vector3UserData::setMetamethod(HSQUIRRELVM vm){
        SQObjectType objectType = sq_gettype(vm, -1);
        if(objectType != OT_FLOAT && objectType != OT_INTEGER) return 0;

        SQFloat val;
        sq_getfloat(vm, -1, &val);

        const SQChar *key;
        sq_getstring(vm, -2, &key);

        enum class TargetType{
            X, Y, Z, None
        };
        TargetType foundType = TargetType::None;
        if(strcmp(key, "x") == 0) foundType = TargetType::X;
        else if(strcmp(key, "y") == 0) foundType = TargetType::Y;
        else if(strcmp(key, "z") == 0) foundType = TargetType::Z;

        if(foundType == TargetType::None){
            sq_pushnull(vm);
            return sq_throwobject(vm);
        }

        Ogre::Vector3 *outVec;
        if(!_readVector3PtrFromUserData(vm, -3, &outVec)){
            //Should not happen.
            assert(false);
            return 0;
        }

        if(foundType == TargetType::X) outVec->x = val;
        else if(foundType == TargetType::Y) outVec->y = val;
        else if(foundType == TargetType::Z) outVec->z = val;

        return 0;
    }

    void Vector3UserData::vector3ToUserData(HSQUIRRELVM vm, const Ogre::Vector3& vec){
        Ogre::Vector3* pointer = (Ogre::Vector3*)sq_newuserdata(vm, sizeof(Ogre::Vector3*));
        *pointer = vec;

        sq_pushobject(vm, vector3DelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, Vector3TypeTag);
    }

    bool Vector3UserData::readVector3FromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Vector3* outVec){
        Ogre::Vector3* vecPtr;
        if(!_readVector3PtrFromUserData(vm, stackInx, &vecPtr)) return false;

        *outVec = *vecPtr;
        return true;
    }

    bool Vector3UserData::_readVector3PtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Vector3** outVec){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return false;
        if(typeTag != Vector3TypeTag){
            *outVec = 0;
            return false;
        }

        //Ogre::Vector3* p = (Ogre::Vector3*)pointer;
        *outVec = (Ogre::Vector3*)pointer;

        return true;
    }
}
