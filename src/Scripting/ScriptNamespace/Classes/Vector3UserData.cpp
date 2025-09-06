#include "Vector3UserData.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "OgreStringConverter.h"

#include <sstream>

namespace AV{

    SQObject Vector3UserData::vector3DelegateTableObject;

    void Vector3UserData::setupTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 11);

        ScriptUtils::addFunction(vm, setMetamethod, "_set");
        ScriptUtils::addFunction(vm, getMetamethod, "_get");
        ScriptUtils::addFunction(vm, unaryMinusMetamethod, "_unm");
        ScriptUtils::addFunction(vm, vector3ToString, "_tostring");
        ScriptUtils::addFunction(vm, addMetamethod, "_add");
        ScriptUtils::addFunction(vm, minusMetamethod, "_sub");
        ScriptUtils::addFunction(vm, multiplyMetamethod, "_mul");
        ScriptUtils::addFunction(vm, divideMetamethod, "_div");
        ScriptUtils::addFunction(vm, vector3Compare, "_cmp");

        ScriptUtils::addFunction(vm, normalise, "normalise");
        ScriptUtils::addFunction(vm, normalisedCopy, "normalisedCopy");
        ScriptUtils::addFunction(vm, distance, "distance", 2, ".u");
        ScriptUtils::addFunction(vm, squaredDistance, "squaredDistance", 2, ".u");
        ScriptUtils::addFunction(vm, crossProduct, "cross", 2, ".u");
        ScriptUtils::addFunction(vm, dotProduct, "dot", 2, ".u");
        ScriptUtils::addFunction(vm, absDotProduct, "absDot", 2, ".u");
        ScriptUtils::addFunction(vm, abs, "abs");
        ScriptUtils::addFunction(vm, makeCeil, "makeCeil", 2, ".u");
        ScriptUtils::addFunction(vm, makeFloor, "makeFloor", 2, ".u");
        ScriptUtils::addFunction(vm, moveTowards, "moveTowards", 3, ".un");
        ScriptUtils::addFunction(vm, perpendicular, "perpendicular");
        ScriptUtils::addFunction(vm, xy, "xy");
        ScriptUtils::addFunction(vm, xz, "xz");
        ScriptUtils::addFunction(vm, copy, "copy");

        sq_resetobject(&vector3DelegateTableObject);
        sq_getstackobj(vm, -1, &vector3DelegateTableObject);
        sq_addref(vm, &vector3DelegateTableObject);
        sq_pop(vm, 1);

        //Create the creation functions.
        sq_pushroottable(vm);

        {
            ScriptUtils::addFunction(vm, createVector3, "Vec3", -1, ".s|nnn");
        }

        sq_pop(vm, 1);
    }

    SQInteger Vector3UserData::createVector3(HSQUIRRELVM vm){
        SQInteger size = sq_gettop(vm);

        SQFloat x, y, z;
        x = y = z = 0.0f;
        switch(size){
            case 1:
                //Construct an empty vector with zeros.
                break;
            case 2:{
                SQObjectType t = sq_gettype(vm, -1);
                if(t == OT_STRING){
                    const SQChar *value;
                    sq_getstring(vm, -1, &value);
                    const Ogre::Vector3 out = Ogre::StringConverter::parseVector3(value);
                    x = out.x;
                    y = out.y;
                    z = out.z;
                }else{
                    assert(t == OT_FLOAT || t == OT_INTEGER);
                    sq_getfloat(vm, -1, &z);
                    sq_getfloat(vm, -1, &y);
                    sq_getfloat(vm, -1, &x);
                }
                break;
            }
            case 4:
                sq_getfloat(vm, -1, &z);
                sq_getfloat(vm, -2, &y);
                sq_getfloat(vm, -3, &x);
                break;
            default:
                return sq_throwerror(vm, "Invalid parameters");
                break;
        }

        vector3ToUserData(vm, Ogre::Vector3(x, y, z));

        return 1;
    }

    SQInteger Vector3UserData::normalise(HSQUIRRELVM vm){
        Ogre::Vector3* obj = 0;
        SCRIPT_ASSERT_RESULT(_readVector3PtrFromUserData(vm, 1, &obj));

        obj->normalise();

        return 0;
    }

    SQInteger Vector3UserData::normalisedCopy(HSQUIRRELVM vm){
        Ogre::Vector3* obj = 0;
        SCRIPT_ASSERT_RESULT(_readVector3PtrFromUserData(vm, 1, &obj));

        const Ogre::Vector3 normVec = obj->normalisedCopy();
        vector3ToUserData(vm, normVec);

        return 1;
    }

    SQInteger Vector3UserData::distance(HSQUIRRELVM vm){
        Ogre::Vector3* obj = 0;
        SCRIPT_ASSERT_RESULT(_readVector3PtrFromUserData(vm, 1, &obj));

        Ogre::Vector3* secondObj = 0;
        SCRIPT_CHECK_RESULT(_readVector3PtrFromUserData(vm, 2, &secondObj));

        const float distance = obj->distance(*secondObj);
        sq_pushfloat(vm, distance);

        return 1;
    }

    SQInteger Vector3UserData::squaredDistance(HSQUIRRELVM vm){
        Ogre::Vector3* obj = 0;
        SCRIPT_ASSERT_RESULT(_readVector3PtrFromUserData(vm, 1, &obj));

        Ogre::Vector3* secondObj = 0;
        SCRIPT_CHECK_RESULT(_readVector3PtrFromUserData(vm, 2, &secondObj));

        const float distance = obj->squaredDistance(*secondObj);
        sq_pushfloat(vm, distance);

        return 1;
    }

    SQInteger Vector3UserData::moveTowards(HSQUIRRELVM vm){
        Ogre::Vector3* current = 0;
        SCRIPT_ASSERT_RESULT(_readVector3PtrFromUserData(vm, 1, &current));

        Ogre::Vector3* target = 0;
        SCRIPT_CHECK_RESULT(_readVector3PtrFromUserData(vm, 2, &target));

        SQFloat maxDistance;
        sq_getfloat(vm, 3, &maxDistance);

        Ogre::Vector3 delta(*target - *current);
        Ogre::Real magnitude = delta.length();
        if(magnitude <= maxDistance || magnitude == 0.0f){
            *current = *target;
            return 0;
        }

        *current += (delta / magnitude * maxDistance);

        return 0;
    }

    SQInteger Vector3UserData::perpendicular(HSQUIRRELVM vm){
        Ogre::Vector3* current = 0;
        SCRIPT_ASSERT_RESULT(_readVector3PtrFromUserData(vm, 1, &current));

        const Ogre::Vector3 perpVec = current->perpendicular();
        vector3ToUserData(vm, perpVec);

        return 1;
    }

    SQInteger Vector3UserData::abs(HSQUIRRELVM vm){
        Ogre::Vector3* current = 0;
        SCRIPT_ASSERT_RESULT(_readVector3PtrFromUserData(vm, 1, &current));

        Ogre::Vector3 absVal = *current;
        absVal.makeAbs();
        vector3ToUserData(vm, absVal);

        return 1;
    }

    SQInteger Vector3UserData::crossProduct(HSQUIRRELVM vm){
        Ogre::Vector3* obj = 0;
        SCRIPT_ASSERT_RESULT(_readVector3PtrFromUserData(vm, 1, &obj));

        Ogre::Vector3* secondObj = 0;
        SCRIPT_CHECK_RESULT(_readVector3PtrFromUserData(vm, 2, &secondObj));

        const Ogre::Vector3 crossProduct = obj->crossProduct(*secondObj);

        vector3ToUserData(vm, crossProduct);

        return 1;
    }

    SQInteger Vector3UserData::dotProduct(HSQUIRRELVM vm){
        Ogre::Vector3* obj = 0;
        SCRIPT_ASSERT_RESULT(_readVector3PtrFromUserData(vm, 1, &obj));

        Ogre::Vector3* secondObj = 0;
        SCRIPT_CHECK_RESULT(_readVector3PtrFromUserData(vm, 2, &secondObj));

        const float dotProduct = obj->dotProduct(*secondObj);

        sq_pushfloat(vm, dotProduct);

        return 1;
    }

    SQInteger Vector3UserData::absDotProduct(HSQUIRRELVM vm){
        Ogre::Vector3* obj = 0;
        SCRIPT_ASSERT_RESULT(_readVector3PtrFromUserData(vm, 1, &obj));

        Ogre::Vector3* secondObj = 0;
        SCRIPT_CHECK_RESULT(_readVector3PtrFromUserData(vm, 2, &secondObj));

        const float dotProduct = obj->absDotProduct(*secondObj);

        sq_pushfloat(vm, dotProduct);

        return 1;
    }

    SQInteger Vector3UserData::makeCeil(HSQUIRRELVM vm){
        Ogre::Vector3* obj = 0;
        SCRIPT_ASSERT_RESULT(_readVector3PtrFromUserData(vm, 1, &obj));

        Ogre::Vector3* secondObj = 0;
        SCRIPT_CHECK_RESULT(_readVector3PtrFromUserData(vm, 2, &secondObj));

        obj->makeCeil(*secondObj);

        return 0;
    }

    SQInteger Vector3UserData::makeFloor(HSQUIRRELVM vm){
        Ogre::Vector3* obj = 0;
        SCRIPT_ASSERT_RESULT(_readVector3PtrFromUserData(vm, 1, &obj));

        Ogre::Vector3* secondObj = 0;
        SCRIPT_CHECK_RESULT(_readVector3PtrFromUserData(vm, 2, &secondObj));

        obj->makeFloor(*secondObj);

        return 0;
    }

    SQInteger Vector3UserData::addMetamethod(HSQUIRRELVM vm){
        return _operatorMetamethod(vm, OperationType::Add);
    }

    SQInteger Vector3UserData::minusMetamethod(HSQUIRRELVM vm){
        return _operatorMetamethod(vm, OperationType::Subtract);
    }

    SQInteger Vector3UserData::multiplyMetamethod(HSQUIRRELVM vm){
        return _operatorMetamethod(vm, OperationType::Multiply);
    }

    SQInteger Vector3UserData::divideMetamethod(HSQUIRRELVM vm){
        return _operatorMetamethod(vm, OperationType::Divide);
    }

    SQInteger Vector3UserData::_operatorMetamethod(HSQUIRRELVM vm, OperationType opType){
        SQObjectType objectType = sq_gettype(vm, -1);
        bool isNumberType = objectType == OT_FLOAT || objectType == OT_INTEGER;
        if(!isNumberType && objectType != OT_USERDATA)
            return sq_throwerror(vm, "Incorrect type passed");

        Ogre::Vector3* obj = 0;
        SCRIPT_ASSERT_RESULT(_readVector3PtrFromUserData(vm, -2, &obj));

        Ogre::Vector3 vecResult;
        if(objectType == OT_USERDATA){
            assert(!isNumberType);
            Ogre::Vector3* foundObj = 0;
            SCRIPT_CHECK_RESULT(_readVector3PtrFromUserData(vm, -1, &foundObj));
            switch(opType){
                case OperationType::Add: vecResult = (*obj) + (*foundObj); break;
                case OperationType::Subtract: vecResult = (*obj) - (*foundObj); break;
                case OperationType::Multiply: vecResult = (*obj) * (*foundObj); break;
                case OperationType::Divide: vecResult = (*obj) / (*foundObj); break;
            }
        }
        else if(isNumberType){
            SQFloat numberValue;
            sq_getfloat(vm, -1, &numberValue);
            switch(opType){
                case OperationType::Add: vecResult = (*obj) + numberValue; break;
                case OperationType::Subtract: vecResult = (*obj) - numberValue; break;
                case OperationType::Multiply: vecResult = (*obj) * numberValue; break;
                case OperationType::Divide: vecResult = (*obj) / numberValue; break;
            }
        }
        else assert(false);

        vector3ToUserData(vm, vecResult);
        return 1;
    }

    SQInteger Vector3UserData::unaryMinusMetamethod(HSQUIRRELVM vm){
        Ogre::Vector3 *outVec;
        SCRIPT_ASSERT_RESULT(_readVector3PtrFromUserData(vm, -1, &outVec));
        vector3ToUserData(vm, -(*outVec));

        return 1;
    }

    SQInteger Vector3UserData::vector3Compare(HSQUIRRELVM vm){
        Ogre::Vector3* first;
        Ogre::Vector3* second;

        SCRIPT_ASSERT_RESULT(_readVector3PtrFromUserData(vm, -2, &first));
        SCRIPT_CHECK_RESULT(_readVector3PtrFromUserData(vm, -1, &second));

        if(*first == *second){
            sq_pushinteger(vm, 0);
        }else{
            sq_pushinteger(vm, 2);
        }
        return 1;
    }

    enum class TargetType{
        X, Y, Z, None
    };
    SQInteger Vector3UserData::getMetamethod(HSQUIRRELVM vm){
        const SQChar *key;
        sq_getstring(vm, -1, &key);

        if(sq_gettype(vm, -1) != OT_STRING){
            return sq_throwerror(vm, "Valid get values for Vec3 are x,y,z");
        }

        TargetType foundType = TargetType::None;
        if(strcmp(key, "x") == 0) foundType = TargetType::X;
        else if(strcmp(key, "y") == 0) foundType = TargetType::Y;
        else if(strcmp(key, "z") == 0) foundType = TargetType::Z;

        if(foundType == TargetType::None){
            return sq_throwerror(vm, "Valid get values for Vec3 are x,y,z");
        }

        Ogre::Vector3 *outVec;
        SCRIPT_ASSERT_RESULT(_readVector3PtrFromUserData(vm, -2, &outVec));

        SQFloat value = 0.0f;
        if(foundType == TargetType::X) value = outVec->x;
        else if(foundType == TargetType::Y) value = outVec->y;
        else if(foundType == TargetType::Z) value = outVec->z;

        sq_pushfloat(vm, value);

        return 1;
    }


    SQInteger Vector3UserData::xy(HSQUIRRELVM vm){
        Ogre::Vector3 *outVec;
        SCRIPT_ASSERT_RESULT(_readVector3PtrFromUserData(vm, 1, &outVec));

        Vector2UserData::vector2ToUserData(vm, Ogre::Vector2(outVec->x, outVec->y));

        return 1;
    }


    SQInteger Vector3UserData::xz(HSQUIRRELVM vm){
        Ogre::Vector3 *outVec;
        SCRIPT_ASSERT_RESULT(_readVector3PtrFromUserData(vm, 1, &outVec));

        Vector2UserData::vector2ToUserData(vm, Ogre::Vector2(outVec->x, outVec->z));

        return 1;
    }


    SQInteger Vector3UserData::copy(HSQUIRRELVM vm){
        Ogre::Vector3 *outVec;
        SCRIPT_ASSERT_RESULT(_readVector3PtrFromUserData(vm, 1, &outVec));

        vector3ToUserData(vm, Ogre::Vector3(*outVec));

        return 1;
    }

    SQInteger Vector3UserData::setMetamethod(HSQUIRRELVM vm){
        SQObjectType objectType = sq_gettype(vm, -1);
        if(objectType != OT_FLOAT && objectType != OT_INTEGER) return sq_throwerror(vm, "Incorrect type passed in assignment");

        SQFloat val;
        sq_getfloat(vm, -1, &val);

        const SQChar *key;
        sq_getstring(vm, -2, &key);

        if(sq_gettype(vm, -2) != OT_STRING){
            return sq_throwerror(vm, "Valid set values for Vec3 are x,y,z");
        }

        TargetType foundType = TargetType::None;
        if(strcmp(key, "x") == 0) foundType = TargetType::X;
        else if(strcmp(key, "y") == 0) foundType = TargetType::Y;
        else if(strcmp(key, "z") == 0) foundType = TargetType::Z;

        if(foundType == TargetType::None){
            return sq_throwerror(vm, "Valid get values for Vec3 are x,y,z");
        }

        Ogre::Vector3 *outVec;
        SCRIPT_ASSERT_RESULT(_readVector3PtrFromUserData(vm, -3, &outVec));

        if(foundType == TargetType::X) outVec->x = val;
        else if(foundType == TargetType::Y) outVec->y = val;
        else if(foundType == TargetType::Z) outVec->z = val;

        return 0;
    }

    SQInteger Vector3UserData::vector3ToString(HSQUIRRELVM vm){
        Ogre::Vector3* outVec;
        SCRIPT_ASSERT_RESULT(_readVector3PtrFromUserData(vm, -1, &outVec));

        std::ostringstream stream;
        stream << *outVec;
        sq_pushstring(vm, _SC(stream.str().c_str()), -1);

        return 1;
    }

    void Vector3UserData::vector3ToUserData(HSQUIRRELVM vm, const Ogre::Vector3& vec){
        Ogre::Vector3* pointer = (Ogre::Vector3*)sq_newuserdata(vm, sizeof(Ogre::Vector3));
        *pointer = vec;

        sq_pushobject(vm, vector3DelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, Vector3TypeTag);
    }

    UserDataGetResult Vector3UserData::readVector3FromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Vector3* outVec){
        Ogre::Vector3* vecPtr;
        UserDataGetResult result = _readVector3PtrFromUserData(vm, stackInx, &vecPtr);
        if(result != USER_DATA_GET_SUCCESS) return result;

        *outVec = *vecPtr;
        return result;
    }

    UserDataGetResult Vector3UserData::_readVector3PtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Vector3** outVec){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != Vector3TypeTag){
            *outVec = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        //Ogre::Vector3* p = (Ogre::Vector3*)pointer;
        *outVec = (Ogre::Vector3*)pointer;

        return USER_DATA_GET_SUCCESS;
    }
}
