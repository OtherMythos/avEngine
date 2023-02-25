#include "ColourValueUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include <sstream>

namespace AV{

    SQObject ColourValueUserData::colourValueDelegateTableObject;

    void ColourValueUserData::setupTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 11);

        ScriptUtils::addFunction(vm, setMetamethod, "_set");
        ScriptUtils::addFunction(vm, getMetamethod, "_get");
        ScriptUtils::addFunction(vm, colourValueToString, "_tostring");
        ScriptUtils::addFunction(vm, addMetamethod, "_add");
        ScriptUtils::addFunction(vm, minusMetamethod, "_sub");
        ScriptUtils::addFunction(vm, multiplyMetamethod, "_mul");
        ScriptUtils::addFunction(vm, divideMetamethod, "_div");
        ScriptUtils::addFunction(vm, colourValueCompare, "_cmp");
        ScriptUtils::addFunction(vm, copy, "copy");

        sq_resetobject(&colourValueDelegateTableObject);
        sq_getstackobj(vm, -1, &colourValueDelegateTableObject);
        sq_addref(vm, &colourValueDelegateTableObject);
        sq_pop(vm, 1);

        //Create the creation functions.
        sq_pushroottable(vm);

        {
            ScriptUtils::addFunction(vm, createColourValue, "ColourValue", 5, ".nnnn");
        }

        sq_pop(vm, 1);
    }

    SQInteger ColourValueUserData::createColourValue(HSQUIRRELVM vm){
        SQInteger size = sq_gettop(vm);

        SQFloat r, g, b, a;
        r = g = b = a = 0.0f;

        sq_getfloat(vm, -1, &a);
        sq_getfloat(vm, -2, &b);
        sq_getfloat(vm, -3, &g);
        sq_getfloat(vm, -4, &r);

        colourValueToUserData(vm, Ogre::ColourValue(r, g, b, a));

        return 1;
    }

    SQInteger ColourValueUserData::addMetamethod(HSQUIRRELVM vm){
        return _operatorMetamethod(vm, OperationType::Add);
    }

    SQInteger ColourValueUserData::minusMetamethod(HSQUIRRELVM vm){
        return _operatorMetamethod(vm, OperationType::Subtract);
    }

    SQInteger ColourValueUserData::multiplyMetamethod(HSQUIRRELVM vm){
        return _operatorMetamethod(vm, OperationType::Multiply);
    }

    SQInteger ColourValueUserData::divideMetamethod(HSQUIRRELVM vm){
        return _operatorMetamethod(vm, OperationType::Divide);
    }

    SQInteger ColourValueUserData::_operatorMetamethod(HSQUIRRELVM vm, OperationType opType){
        SQObjectType objectType = sq_gettype(vm, -1);
        bool isNumberType = objectType == OT_FLOAT || objectType == OT_INTEGER;
        if(!isNumberType && objectType != OT_USERDATA)
            return sq_throwerror(vm, "Incorrect type passed");

        Ogre::ColourValue* obj = 0;
        SCRIPT_ASSERT_RESULT(_readColourValuePtrFromUserData(vm, -2, &obj));

        Ogre::ColourValue vecResult;
        if(objectType == OT_USERDATA){
            assert(!isNumberType);
            Ogre::ColourValue* foundObj = 0;
            SCRIPT_CHECK_RESULT(_readColourValuePtrFromUserData(vm, -1, &foundObj));
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
                case OperationType::Add: vecResult = (*obj) + Ogre::ColourValue(numberValue, numberValue, numberValue, numberValue); break;
                case OperationType::Subtract: vecResult = (*obj) - Ogre::ColourValue(numberValue, numberValue, numberValue, numberValue); break;
                case OperationType::Multiply: vecResult = (*obj) * numberValue; break;
                case OperationType::Divide: vecResult = (*obj) / numberValue; break;
            }
        }
        else assert(false);

        colourValueToUserData(vm, vecResult);
        return 1;
    }

    SQInteger ColourValueUserData::colourValueCompare(HSQUIRRELVM vm){
        Ogre::ColourValue* first;
        Ogre::ColourValue* second;

        SCRIPT_ASSERT_RESULT(_readColourValuePtrFromUserData(vm, -2, &first));
        SCRIPT_CHECK_RESULT(_readColourValuePtrFromUserData(vm, -1, &second));

        if(*first == *second){
            sq_pushinteger(vm, 0);
        }else{
            sq_pushinteger(vm, 2);
        }
        return 1;
    }

    enum class TargetType{
        R, G, B, A, None
    };
    SQInteger ColourValueUserData::getMetamethod(HSQUIRRELVM vm){
        const SQChar *key;
        sq_getstring(vm, -1, &key);

        if(sq_gettype(vm, -1) != OT_STRING){
            return sq_throwerror(vm, "Valid get values for ColourValues are r,g,b,a");
        }

        TargetType foundType = TargetType::None;
        if(strcmp(key, "r") == 0) foundType = TargetType::R;
        else if(strcmp(key, "g") == 0) foundType = TargetType::G;
        else if(strcmp(key, "b") == 0) foundType = TargetType::B;
        else if(strcmp(key, "a") == 0) foundType = TargetType::A;

        if(foundType == TargetType::None){
            return sq_throwerror(vm, "Valid get values for ColourValues are r,g,b,a");
        }

        Ogre::ColourValue *outCol;
        SCRIPT_ASSERT_RESULT(_readColourValuePtrFromUserData(vm, -2, &outCol));

        SQFloat value = 0.0f;
        if(foundType == TargetType::R) value = outCol->r;
        else if(foundType == TargetType::G) value = outCol->g;
        else if(foundType == TargetType::B) value = outCol->b;
        else if(foundType == TargetType::A) value = outCol->a;

        sq_pushfloat(vm, value);

        return 1;
    }

    SQInteger ColourValueUserData::setMetamethod(HSQUIRRELVM vm){
        SQObjectType objectType = sq_gettype(vm, -1);
        if(objectType != OT_FLOAT && objectType != OT_INTEGER) return sq_throwerror(vm, "Incorrect type passed in assignment");

        SQFloat val;
        sq_getfloat(vm, -1, &val);

        const SQChar *key;
        sq_getstring(vm, -2, &key);

        if(sq_gettype(vm, -2) != OT_STRING){
            return sq_throwerror(vm, "Valid set values for ColourValues are x,y,z");
        }

        TargetType foundType = TargetType::None;
        if(strcmp(key, "r") == 0) foundType = TargetType::R;
        else if(strcmp(key, "g") == 0) foundType = TargetType::G;
        else if(strcmp(key, "b") == 0) foundType = TargetType::B;
        else if(strcmp(key, "a") == 0) foundType = TargetType::A;

        if(foundType == TargetType::None){
            return sq_throwerror(vm, "Valid get values for ColourValues are x,y,z");
        }

        Ogre::ColourValue *outCol;
        SCRIPT_ASSERT_RESULT(_readColourValuePtrFromUserData(vm, -3, &outCol));

        if(foundType == TargetType::R) outCol->r = val;
        else if(foundType == TargetType::G) outCol->g = val;
        else if(foundType == TargetType::B) outCol->b = val;
        else if(foundType == TargetType::A) outCol->a = val;

        return 0;
    }

    SQInteger ColourValueUserData::colourValueToString(HSQUIRRELVM vm){
        Ogre::ColourValue* outCol;
        SCRIPT_ASSERT_RESULT(_readColourValuePtrFromUserData(vm, -1, &outCol));

        std::ostringstream stream;
        stream << *outCol;
        sq_pushstring(vm, _SC(stream.str().c_str()), -1);

        return 1;
    }

    SQInteger ColourValueUserData::copy(HSQUIRRELVM vm){
        Ogre::ColourValue* outCol;
        SCRIPT_ASSERT_RESULT(_readColourValuePtrFromUserData(vm, -1, &outCol));

        Ogre::ColourValue colVal(*outCol);
        colourValueToUserData(vm, colVal);

        return 1;
    }

    void ColourValueUserData::colourValueToUserData(HSQUIRRELVM vm, const Ogre::ColourValue& vec){
        Ogre::ColourValue* pointer = (Ogre::ColourValue*)sq_newuserdata(vm, sizeof(Ogre::ColourValue));
        *pointer = vec;

        sq_pushobject(vm, colourValueDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, ColourValueTypeTag);
    }

    UserDataGetResult ColourValueUserData::readColourValueFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::ColourValue* outCol){
        Ogre::ColourValue* vecPtr;
        UserDataGetResult result = _readColourValuePtrFromUserData(vm, stackInx, &vecPtr);
        if(result != USER_DATA_GET_SUCCESS) return result;

        *outCol = *vecPtr;
        return result;
    }

    UserDataGetResult ColourValueUserData::_readColourValuePtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::ColourValue** outCol){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != ColourValueTypeTag){
            *outCol = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        //Ogre::ColourValue* p = (Ogre::ColourValue*)pointer;
        *outCol = (Ogre::ColourValue*)pointer;

        return USER_DATA_GET_SUCCESS;
    }
}
