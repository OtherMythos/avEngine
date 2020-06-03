#include "Vector3UserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

namespace AV{

    SQObject Vector3UserData::vector3DelegateTableObject;

    void Vector3UserData::setupTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 1);

        ScriptUtils::addFunction(vm, setMetamethod, "_set");
        ScriptUtils::addFunction(vm, getMetamethod, "_get");
        ScriptUtils::addFunction(vm, vector3ToString, "_tostring");


        sq_resetobject(&vector3DelegateTableObject);
        sq_getstackobj(vm, -1, &vector3DelegateTableObject);
        sq_addref(vm, &vector3DelegateTableObject);
        sq_pop(vm, 1);

        //Create the creation functions.
        sq_pushroottable(vm);

        {
            ScriptUtils::addFunction(vm, createVector3, "Vec3", 4, ".nnn");
        }

        sq_pop(vm, 1);
    }

    SQInteger Vector3UserData::createVector3(HSQUIRRELVM vm){
        SQFloat x, y, z;
        x = y = z = 0.0f;
        sq_getfloat(vm, -1, &z);
        sq_getfloat(vm, -2, &y);
        sq_getfloat(vm, -3, &x);

        vector3ToUserData(vm, Ogre::Vector3(x, y, z));

        return 1;
    }

    enum class TargetType{
        X, Y, Z, None
    };
    SQInteger Vector3UserData::getMetamethod(HSQUIRRELVM vm){
        const SQChar *key;
        sq_getstring(vm, -1, &key);

        TargetType foundType = TargetType::None;
        if(strcmp(key, "x") == 0) foundType = TargetType::X;
        else if(strcmp(key, "y") == 0) foundType = TargetType::Y;
        else if(strcmp(key, "z") == 0) foundType = TargetType::Z;

        if(foundType == TargetType::None){
            sq_pushnull(vm);
            return sq_throwobject(vm);
        }

        Ogre::Vector3 *outVec;
        if(!_readVector3PtrFromUserData(vm, -2, &outVec)){
            //Should not happen.
            assert(false);
            return 0;
        }

        SQFloat value = 0.0f;
        if(foundType == TargetType::X) value = outVec->x;
        else if(foundType == TargetType::Y) value = outVec->y;
        else if(foundType == TargetType::Z) value = outVec->z;

        sq_pushfloat(vm, value);

        return 1;
    }

    SQInteger Vector3UserData::setMetamethod(HSQUIRRELVM vm){
        SQObjectType objectType = sq_gettype(vm, -1);
        if(objectType != OT_FLOAT && objectType != OT_INTEGER) return sq_throwerror(vm, "Incorrect type passed in assignment");

        SQFloat val;
        sq_getfloat(vm, -1, &val);

        const SQChar *key;
        sq_getstring(vm, -2, &key);

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

    SQInteger Vector3UserData::vector3ToString(HSQUIRRELVM vm){
        Ogre::Vector3* outVec;
        bool success = _readVector3PtrFromUserData(vm, -1, &outVec);
        if(!success){
            sq_pushstring(vm, "", -1);
            return 1;
        }

        std::ostringstream stream;
        stream << *outVec;
        sq_pushstring(vm, _SC(stream.str().c_str()), -1);

        return 1;
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
