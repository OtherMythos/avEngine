#include "QuaternionUserData.h"

#include <sstream>
#include "Scripting/ScriptObjectTypeTags.h"

#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"

namespace AV{

    SQObject QuaternionUserData::quaternionDelegateTableObject;

    void QuaternionUserData::setupTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 11);

        ScriptUtils::addFunction(vm, setMetamethod, "_set");
        ScriptUtils::addFunction(vm, getMetamethod, "_get");
        ScriptUtils::addFunction(vm, unaryMinusMetamethod, "_unm");
        ScriptUtils::addFunction(vm, quaternionToString, "_tostring");
        ScriptUtils::addFunction(vm, addMetamethod, "_add");
        ScriptUtils::addFunction(vm, minusMetamethod, "_sub");
        ScriptUtils::addFunction(vm, multiplyMetamethod, "_mul");
        ScriptUtils::addFunction(vm, copy, "copy");
        ScriptUtils::addFunction(vm, quaternionCompare, "_cmp");

        ScriptUtils::addFunction(vm, slerp, "slerp", -3, ".nub");
        ScriptUtils::addFunction(vm, nlerp, "nlerp", -3, ".nub");

        sq_resetobject(&quaternionDelegateTableObject);
        sq_getstackobj(vm, -1, &quaternionDelegateTableObject);
        sq_addref(vm, &quaternionDelegateTableObject);
        sq_pop(vm, 1);

        //Create the creation functions.
        sq_pushroottable(vm);

        {
            //x, y, z, w
            ScriptUtils::addFunction(vm, createQuaternion, "Quat", -1, ".nn|unn");
        }

        sq_pop(vm, 1);
    }

    SQInteger QuaternionUserData::createQuaternion(HSQUIRRELVM vm){

        SQInteger top = sq_gettop(vm);
        SQFloat x, y, z, w;
        x = y = z = 0.0;
        w = 1.0;
        if(top == 5){
            w = x = y = z = 0.0f;
            sq_getfloat(vm, -1, &w);
            sq_getfloat(vm, -2, &z);
            sq_getfloat(vm, -3, &y);
            sq_getfloat(vm, -4, &x);
        }else if(top <= 1){
            //Don't error just use the default values.
        }else if(top == 3){
            SQFloat rad;
            sq_getfloat(vm, 2, &rad);
            Ogre::Vector3 val;
            SCRIPT_CHECK_RESULT(Vector3UserData::readVector3FromUserData(vm, 3, &val));

            Ogre::Quaternion quat(Ogre::Radian(rad), val);

            quaternionToUserData(vm, quat);
            return 1;
        }else{
            return sq_throwerror(vm, "Invalid parameters. Expected nnnn or empty.");
        }

        quaternionToUserData(vm, Ogre::Quaternion(w, x, y, z));

        return 1;
    }

    SQInteger QuaternionUserData::_slerpNlerp(HSQUIRRELVM vm, bool slerp){
        Ogre::Quaternion* first = 0;
        SCRIPT_ASSERT_RESULT(QuaternionUserData::_readQuaternionPtrFromUserData(vm, 1, &first));

        SQFloat radian;
        sq_getfloat(vm, 2, &radian);

        Ogre::Quaternion* second = 0;
        SCRIPT_CHECK_RESULT(QuaternionUserData::_readQuaternionPtrFromUserData(vm, 3, &second));

        SQBool shortestPath = false;
        if(sq_gettop(vm) >= 4){
            sq_getbool(vm, 4, &shortestPath);
        }

        Ogre::Quaternion out;
        if(slerp){
            out = Ogre::Quaternion::Slerp(radian, *first, *second, shortestPath);
        }else{
            out = Ogre::Quaternion::nlerp(radian, *first, *second, shortestPath);
        }
        QuaternionUserData::quaternionToUserData(vm, out);

        return 1;
    }

    SQInteger QuaternionUserData::copy(HSQUIRRELVM vm){
        Ogre::Quaternion* first = 0;
        SCRIPT_ASSERT_RESULT(QuaternionUserData::_readQuaternionPtrFromUserData(vm, 1, &first));

        Ogre::Quaternion quat(*first);
        quaternionToUserData(vm, quat);

        return 1;
    }

    SQInteger QuaternionUserData::slerp(HSQUIRRELVM vm){
        return _slerpNlerp(vm, true);
    }

    SQInteger QuaternionUserData::nlerp(HSQUIRRELVM vm){
        return _slerpNlerp(vm, false);
    }

    SQInteger QuaternionUserData::quaternionCompare(HSQUIRRELVM vm){
        Ogre::Quaternion* first;
        Ogre::Quaternion* second;

        SCRIPT_ASSERT_RESULT(_readQuaternionPtrFromUserData(vm, -2, &first));
        SCRIPT_CHECK_RESULT(_readQuaternionPtrFromUserData(vm, -1, &second));

        if(*first == *second){
            sq_pushinteger(vm, 0);
        }else{
            sq_pushinteger(vm, 2);
        }
        return 1;
    }

    SQInteger QuaternionUserData::addMetamethod(HSQUIRRELVM vm){
        return _operatorMetamethod(vm, OperationType::Add);
    }

    SQInteger QuaternionUserData::minusMetamethod(HSQUIRRELVM vm){
        return _operatorMetamethod(vm, OperationType::Subtract);
    }

    SQInteger QuaternionUserData::multiplyMetamethod(HSQUIRRELVM vm){
        return _operatorMultiplyMetamethod(vm);
    }

    SQInteger QuaternionUserData::_operatorMultiplyMetamethod(HSQUIRRELVM vm){
        SQObjectType objectType = sq_gettype(vm, -1);
        if(objectType != OT_USERDATA){
            return sq_throwerror(vm, "Incorrect type passed");
        }

        Ogre::Quaternion* obj = 0;
        SCRIPT_ASSERT_RESULT(_readQuaternionPtrFromUserData(vm, -2, &obj));

        SQUserPointer type;
        bool failed = SQ_FAILED(sq_gettypetag(vm, -1, &type));
        //Should be a user data, should have a type tag.
        assert(!failed);

        if(type == Vector3TypeTag){
            Ogre::Vector3 val;
            SCRIPT_ASSERT_RESULT(Vector3UserData::readVector3FromUserData(vm, -1, &val));
            Ogre::Vector3 result = *obj * val;
            Vector3UserData::vector3ToUserData(vm, result);
        }
        else if(type == QuaternionTypeTag){
            Ogre::Quaternion* foundObj = 0;
            SCRIPT_CHECK_RESULT(_readQuaternionPtrFromUserData(vm, -1, &foundObj));
            Ogre::Quaternion result = (*obj) * (*foundObj);
            quaternionToUserData(vm, result);
        }
        else{
            return sq_throwerror(vm, "Multiply must be a vector3 or quaternion.");
        }

        return 1;
    }

    SQInteger QuaternionUserData::_operatorMetamethod(HSQUIRRELVM vm, OperationType opType){
        SQObjectType objectType = sq_gettype(vm, -1);
        if(objectType != OT_USERDATA){
            return sq_throwerror(vm, "Incorrect type passed");
        }

        Ogre::Quaternion* obj = 0;
        SCRIPT_ASSERT_RESULT(_readQuaternionPtrFromUserData(vm, -2, &obj));

        Ogre::Quaternion quatResult;
        Ogre::Quaternion* foundObj = 0;
        SCRIPT_CHECK_RESULT(_readQuaternionPtrFromUserData(vm, -1, &foundObj));
        switch(opType){
            case OperationType::Add: quatResult = (*obj) + (*foundObj); break;
            case OperationType::Subtract: quatResult = (*obj) - (*foundObj); break;
        }

        quaternionToUserData(vm, quatResult);
        return 1;
    }

    SQInteger QuaternionUserData::unaryMinusMetamethod(HSQUIRRELVM vm){
        Ogre::Quaternion *outQuat;
        SCRIPT_ASSERT_RESULT(_readQuaternionPtrFromUserData(vm, -1, &outQuat));
        quaternionToUserData(vm, -(*outQuat));

        return 1;
    }

    enum class TargetType{
        X, Y, Z, W, None
    };
    SQInteger QuaternionUserData::getMetamethod(HSQUIRRELVM vm){
        const SQChar *key;
        sq_getstring(vm, -1, &key);

        if(sq_gettype(vm, -1) != OT_STRING){
            return sq_throwerror(vm, "Valid get values for Quaternion are x,y,z,w");
        }

        TargetType foundType = TargetType::None;
        if(strcmp(key, "x") == 0) foundType = TargetType::X;
        else if(strcmp(key, "y") == 0) foundType = TargetType::Y;
        else if(strcmp(key, "z") == 0) foundType = TargetType::Z;
        else if(strcmp(key, "w") == 0) foundType = TargetType::W;

        if(foundType == TargetType::None){
            return sq_throwerror(vm, "Valid get values for Quaternion are x,y,z,w");
        }

        Ogre::Quaternion *outQuat;
        SCRIPT_ASSERT_RESULT(_readQuaternionPtrFromUserData(vm, -2, &outQuat));

        SQFloat value = 0.0f;
        if(foundType == TargetType::X) value = outQuat->x;
        else if(foundType == TargetType::Y) value = outQuat->y;
        else if(foundType == TargetType::Z) value = outQuat->z;
        else if(foundType == TargetType::W) value = outQuat->w;

        sq_pushfloat(vm, value);

        return 1;
    }

    SQInteger QuaternionUserData::setMetamethod(HSQUIRRELVM vm){
        SQObjectType objectType = sq_gettype(vm, -1);
        if(objectType != OT_FLOAT && objectType != OT_INTEGER) return sq_throwerror(vm, "Incorrect type passed in assignment");

        SQFloat val;
        sq_getfloat(vm, -1, &val);

        const SQChar *key;
        sq_getstring(vm, -2, &key);

        if(sq_gettype(vm, -2) != OT_STRING){
            return sq_throwerror(vm, "Valid set values for Quaternion are x,y,z,w");
        }

        TargetType foundType = TargetType::None;
        if(strcmp(key, "x") == 0) foundType = TargetType::X;
        else if(strcmp(key, "y") == 0) foundType = TargetType::Y;
        else if(strcmp(key, "z") == 0) foundType = TargetType::Z;
        else if(strcmp(key, "w") == 0) foundType = TargetType::W;

        if(foundType == TargetType::None){
            return sq_throwerror(vm, "Valid set values for Quaternion are x,y,z,w");
        }

        Ogre::Quaternion *outQuat;
        SCRIPT_ASSERT_RESULT(_readQuaternionPtrFromUserData(vm, -3, &outQuat));

        if(foundType == TargetType::X) outQuat->x = val;
        else if(foundType == TargetType::Y) outQuat->y = val;
        else if(foundType == TargetType::Z) outQuat->z = val;
        else if(foundType == TargetType::W) outQuat->w = val;

        return 0;
    }

    SQInteger QuaternionUserData::quaternionToString(HSQUIRRELVM vm){
        Ogre::Quaternion* outQuat;
        SCRIPT_ASSERT_RESULT(_readQuaternionPtrFromUserData(vm, -1, &outQuat));

        std::ostringstream stream;
        stream << *outQuat;
        sq_pushstring(vm, _SC(stream.str().c_str()), -1);

        return 1;
    }

    void QuaternionUserData::quaternionToUserData(HSQUIRRELVM vm, const Ogre::Quaternion& quat){
        Ogre::Quaternion* pointer = (Ogre::Quaternion*)sq_newuserdata(vm, sizeof(Ogre::Quaternion));
        *pointer = quat;

        sq_pushobject(vm, quaternionDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, QuaternionTypeTag);
    }

    UserDataGetResult QuaternionUserData::readQuaternionFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Quaternion* outQuat){
        Ogre::Quaternion* quatPtr;
        UserDataGetResult result = _readQuaternionPtrFromUserData(vm, stackInx, &quatPtr);
        if(result != USER_DATA_GET_SUCCESS) return result;

        *outQuat = *quatPtr;
        return result;
    }

    UserDataGetResult QuaternionUserData::_readQuaternionPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Quaternion** outQuat){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != QuaternionTypeTag){
            *outQuat = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outQuat = (Ogre::Quaternion*)pointer;

        return USER_DATA_GET_SUCCESS;
    }
}
