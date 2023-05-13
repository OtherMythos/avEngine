#include "Vector2UserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include <sstream>

namespace AV{

    SQObject Vector2UserData::vector2DelegateTableObject;

    void Vector2UserData::setupTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 11);

        ScriptUtils::addFunction(vm, setMetamethod, "_set");
        ScriptUtils::addFunction(vm, getMetamethod, "_get");
        ScriptUtils::addFunction(vm, unaryMinusMetamethod, "_unm");
        ScriptUtils::addFunction(vm, vector2ToString, "_tostring");
        ScriptUtils::addFunction(vm, addMetamethod, "_add");
        ScriptUtils::addFunction(vm, minusMetamethod, "_sub");
        ScriptUtils::addFunction(vm, multiplyMetamethod, "_mul");
        ScriptUtils::addFunction(vm, divideMetamethod, "_div");
        ScriptUtils::addFunction(vm, vector2Compare, "_cmp");

        ScriptUtils::addFunction(vm, normalise, "normalise");
        ScriptUtils::addFunction(vm, normalisedCopy, "normalisedCopy");
        ScriptUtils::addFunction(vm, distance, "distance", 2, ".u");
        ScriptUtils::addFunction(vm, crossProduct, "cross", 2, ".u");
        ScriptUtils::addFunction(vm, makeCeil, "makeCeil", 2, ".u");
        ScriptUtils::addFunction(vm, makeFloor, "makeFloor", 2, ".u");
        ScriptUtils::addFunction(vm, perpendicular, "perpendicular");
        ScriptUtils::addFunction(vm, copy, "copy");

        sq_resetobject(&vector2DelegateTableObject);
        sq_getstackobj(vm, -1, &vector2DelegateTableObject);
        sq_addref(vm, &vector2DelegateTableObject);
        sq_pop(vm, 1);

        //Create the creation functions.
        sq_pushroottable(vm);

        {
            ScriptUtils::addFunction(vm, createVector2, "Vec2", -1, ".nn");
        }

        sq_pop(vm, 1);
    }

    SQInteger Vector2UserData::createVector2(HSQUIRRELVM vm){
        SQInteger size = sq_gettop(vm);

        SQFloat x, y;
        x = y = 0.0f;
        switch(size){
            case 1:
                //Construct an empty vector with zeros.
                break;
            case 2:
                sq_getfloat(vm, -1, &y);
                sq_getfloat(vm, -1, &x);
                break;
            case 3:
                sq_getfloat(vm, -1, &y);
                sq_getfloat(vm, -2, &x);
                break;
            default:
                return sq_throwerror(vm, "Invalid parameters");
                break;
        }

        vector2ToUserData(vm, Ogre::Vector2(x, y));

        return 1;
    }

    SQInteger Vector2UserData::copy(HSQUIRRELVM vm){
        Ogre::Vector2 *outVec;
        SCRIPT_ASSERT_RESULT(_readVector2PtrFromUserData(vm, 1, &outVec));

        vector2ToUserData(vm, Ogre::Vector2(*outVec));

        return 1;
    }

    SQInteger Vector2UserData::normalise(HSQUIRRELVM vm){
        Ogre::Vector2* obj = 0;
        SCRIPT_ASSERT_RESULT(_readVector2PtrFromUserData(vm, 1, &obj));

        obj->normalise();

        return 0;
    }

    SQInteger Vector2UserData::normalisedCopy(HSQUIRRELVM vm){
        Ogre::Vector2* obj = 0;
        SCRIPT_ASSERT_RESULT(_readVector2PtrFromUserData(vm, 1, &obj));

        const Ogre::Vector2 normVec = obj->normalisedCopy();
        vector2ToUserData(vm, normVec);

        return 1;
    }

    SQInteger Vector2UserData::distance(HSQUIRRELVM vm){
        Ogre::Vector2* obj = 0;
        SCRIPT_ASSERT_RESULT(_readVector2PtrFromUserData(vm, 1, &obj));

        Ogre::Vector2* secondObj = 0;
        SCRIPT_CHECK_RESULT(_readVector2PtrFromUserData(vm, 2, &secondObj));

        const float distance = obj->distance(*secondObj);
        sq_pushfloat(vm, distance);

        return 1;
    }

    SQInteger Vector2UserData::crossProduct(HSQUIRRELVM vm){
        Ogre::Vector2* obj = 0;
        SCRIPT_ASSERT_RESULT(_readVector2PtrFromUserData(vm, 1, &obj));

        Ogre::Vector2* secondObj = 0;
        SCRIPT_CHECK_RESULT(_readVector2PtrFromUserData(vm, 2, &secondObj));

        const Ogre::Real crossProduct = obj->crossProduct(*secondObj);

        sq_pushfloat(vm, crossProduct);

        return 1;
    }

    SQInteger Vector2UserData::makeCeil(HSQUIRRELVM vm){
        Ogre::Vector2* obj = 0;
        SCRIPT_ASSERT_RESULT(_readVector2PtrFromUserData(vm, 1, &obj));

        Ogre::Vector2* secondObj = 0;
        SCRIPT_CHECK_RESULT(_readVector2PtrFromUserData(vm, 2, &secondObj));

        obj->makeCeil(*secondObj);

        return 0;
    }

    SQInteger Vector2UserData::makeFloor(HSQUIRRELVM vm){
        Ogre::Vector2* obj = 0;
        SCRIPT_ASSERT_RESULT(_readVector2PtrFromUserData(vm, 1, &obj));

        Ogre::Vector2* secondObj = 0;
        SCRIPT_CHECK_RESULT(_readVector2PtrFromUserData(vm, 2, &secondObj));

        obj->makeFloor(*secondObj);

        return 0;
    }


    SQInteger Vector2UserData::perpendicular(HSQUIRRELVM vm){
        Ogre::Vector2* current = 0;
        SCRIPT_ASSERT_RESULT(_readVector2PtrFromUserData(vm, 1, &current));

        const Ogre::Vector2 perpVec = current->perpendicular();
        vector2ToUserData(vm, perpVec);

        return 1;
    }

    SQInteger Vector2UserData::addMetamethod(HSQUIRRELVM vm){
        return _operatorMetamethod(vm, OperationType::Add);
    }

    SQInteger Vector2UserData::minusMetamethod(HSQUIRRELVM vm){
        return _operatorMetamethod(vm, OperationType::Subtract);
    }

    SQInteger Vector2UserData::multiplyMetamethod(HSQUIRRELVM vm){
        return _operatorMetamethod(vm, OperationType::Multiply);
    }

    SQInteger Vector2UserData::divideMetamethod(HSQUIRRELVM vm){
        return _operatorMetamethod(vm, OperationType::Divide);
    }

    SQInteger Vector2UserData::_operatorMetamethod(HSQUIRRELVM vm, OperationType opType){
        SQObjectType objectType = sq_gettype(vm, -1);
        bool isNumberType = objectType == OT_FLOAT || objectType == OT_INTEGER;
        if(!isNumberType && objectType != OT_USERDATA)
            return sq_throwerror(vm, "Incorrect type passed");

        Ogre::Vector2* obj = 0;
        SCRIPT_ASSERT_RESULT(_readVector2PtrFromUserData(vm, -2, &obj));

        Ogre::Vector2 vecResult;
        if(objectType == OT_USERDATA){
            assert(!isNumberType);
            Ogre::Vector2* foundObj = 0;
            SCRIPT_CHECK_RESULT(_readVector2PtrFromUserData(vm, -1, &foundObj));
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

        vector2ToUserData(vm, vecResult);
        return 1;
    }

    SQInteger Vector2UserData::unaryMinusMetamethod(HSQUIRRELVM vm){
        Ogre::Vector2 *outVec;
        SCRIPT_ASSERT_RESULT(_readVector2PtrFromUserData(vm, -1, &outVec));
        vector2ToUserData(vm, -(*outVec));

        return 1;
    }

    SQInteger Vector2UserData::vector2Compare(HSQUIRRELVM vm){
        Ogre::Vector2* first;
        Ogre::Vector2* second;

        SCRIPT_ASSERT_RESULT(_readVector2PtrFromUserData(vm, -2, &first));
        SCRIPT_CHECK_RESULT(_readVector2PtrFromUserData(vm, -1, &second));

        if(*first == *second){
            sq_pushinteger(vm, 0);
        }else{
            sq_pushinteger(vm, 2);
        }
        return 1;
    }

    enum class TargetType{
        X, Y, None
    };
    SQInteger Vector2UserData::getMetamethod(HSQUIRRELVM vm){
        const SQChar *key;
        sq_getstring(vm, -1, &key);

        if(sq_gettype(vm, -1) != OT_STRING){
            return sq_throwerror(vm, "Valid get values for Vec2 are x,y");
        }

        TargetType foundType = TargetType::None;
        if(strcmp(key, "x") == 0) foundType = TargetType::X;
        else if(strcmp(key, "y") == 0) foundType = TargetType::Y;

        if(foundType == TargetType::None){
            return sq_throwerror(vm, "Valid get values for Vec2 are x,y");
        }

        Ogre::Vector2 *outVec;
        SCRIPT_ASSERT_RESULT(_readVector2PtrFromUserData(vm, -2, &outVec));

        SQFloat value = 0.0f;
        if(foundType == TargetType::X) value = outVec->x;
        else if(foundType == TargetType::Y) value = outVec->y;

        sq_pushfloat(vm, value);

        return 1;
    }

    SQInteger Vector2UserData::setMetamethod(HSQUIRRELVM vm){
        SQObjectType objectType = sq_gettype(vm, -1);
        if(objectType != OT_FLOAT && objectType != OT_INTEGER) return sq_throwerror(vm, "Incorrect type passed in assignment");

        SQFloat val;
        sq_getfloat(vm, -1, &val);

        const SQChar *key;
        sq_getstring(vm, -2, &key);

        if(sq_gettype(vm, -2) != OT_STRING){
            return sq_throwerror(vm, "Valid set values for Vec2 are x,y");
        }

        TargetType foundType = TargetType::None;
        if(strcmp(key, "x") == 0) foundType = TargetType::X;
        else if(strcmp(key, "y") == 0) foundType = TargetType::Y;

        if(foundType == TargetType::None){
            return sq_throwerror(vm, "Valid set values for Vec2 are x,y");
        }

        Ogre::Vector2 *outVec;
        SCRIPT_ASSERT_RESULT(_readVector2PtrFromUserData(vm, -3, &outVec));

        if(foundType == TargetType::X) outVec->x = val;
        else if(foundType == TargetType::Y) outVec->y = val;

        return 0;
    }

    SQInteger Vector2UserData::vector2ToString(HSQUIRRELVM vm){
        Ogre::Vector2* outVec;
        SCRIPT_ASSERT_RESULT(_readVector2PtrFromUserData(vm, -1, &outVec));

        std::ostringstream stream;
        stream << *outVec;
        sq_pushstring(vm, _SC(stream.str().c_str()), -1);

        return 1;
    }

    void Vector2UserData::vector2ToUserData(HSQUIRRELVM vm, const Ogre::Vector2& vec){
        Ogre::Vector2* pointer = (Ogre::Vector2*)sq_newuserdata(vm, sizeof(Ogre::Vector2));
        *pointer = vec;

        sq_pushobject(vm, vector2DelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, Vector2TypeTag);
    }

    UserDataGetResult Vector2UserData::readVector2FromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Vector2* outVec){
        Ogre::Vector2* vecPtr;
        UserDataGetResult result = _readVector2PtrFromUserData(vm, stackInx, &vecPtr);
        if(result != USER_DATA_GET_SUCCESS) return result;

        *outVec = *vecPtr;
        return result;
    }

    UserDataGetResult Vector2UserData::_readVector2PtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Vector2** outVec){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != Vector2TypeTag){
            *outVec = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outVec = (Ogre::Vector2*)pointer;

        return USER_DATA_GET_SUCCESS;
    }
}
