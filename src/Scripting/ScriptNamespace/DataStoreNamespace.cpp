#include "DataStoreNamespace.h"

#include "System/DataStore/DataStore.h"

namespace AV{
    SQInteger DataStoreNamespace::getInt(HSQUIRRELVM vm){
        const SQChar* key;
        sq_getstring(vm, -1, &key);

        int outVal;
        DataStore* ds = DataStore::getInstance();
        if(!ds || !ds->getInt(key, outVal)) return 0;

        sq_pushinteger(vm, outVal);
        return 1;
    }

    SQInteger DataStoreNamespace::getFloat(HSQUIRRELVM vm){
        const SQChar* key;
        sq_getstring(vm, -1, &key);

        float outVal;
        DataStore* ds = DataStore::getInstance();
        if(!ds || !ds->getFloat(key, outVal)) return 0;

        sq_pushfloat(vm, outVal);
        return 1;
    }

    SQInteger DataStoreNamespace::getBool(HSQUIRRELVM vm){
        const SQChar* key;
        sq_getstring(vm, -1, &key);

        bool outVal;
        DataStore* ds = DataStore::getInstance();
        if(!ds || !ds->getBool(key, outVal)) return 0;

        sq_pushbool(vm, outVal);
        return 1;
    }

    SQInteger DataStoreNamespace::getString(HSQUIRRELVM vm){
        const SQChar* key;
        sq_getstring(vm, -1, &key);

        std::string outVal;
        DataStore* ds = DataStore::getInstance();
        if(!ds || !ds->getString(key, outVal)) return 0;

        sq_pushstring(vm, outVal.c_str(), -1);
        return 1;
    }

    SQInteger DataStoreNamespace::setValue(HSQUIRRELVM vm){
        SQObjectType objectType = sq_gettype(vm, -1);

        const SQChar* key;
        sq_getstring(vm, -2, &key);

        DataStore* ds = DataStore::getInstance();
        if(!ds){
            sq_pushbool(vm, false);
            return 1;
        }

        switch(objectType){
            case OT_INTEGER:{
                SQInteger val;
                sq_getinteger(vm, -1, &val);
                ds->setInt(key, (int)val);
                break;
            }
            case OT_FLOAT:{
                SQFloat val;
                sq_getfloat(vm, -1, &val);
                ds->setFloat(key, (float)val);
                break;
            }
            case OT_BOOL:{
                SQBool val;
                sq_getbool(vm, -1, &val);
                ds->setBool(key, val != 0);
                break;
            }
            case OT_STRING:{
                const SQChar* val;
                sq_getstring(vm, -1, &val);
                ds->setString(key, val);
                break;
            }
            case OT_NULL:{
                ds->remove(key);
                break;
            }
            default:{
                sq_pushbool(vm, false);
                return 1;
            }
        }

        sq_pushbool(vm, true);
        return 1;
    }

    SQInteger DataStoreNamespace::removeValue(HSQUIRRELVM vm){
        const SQChar* key;
        sq_getstring(vm, -1, &key);

        DataStore* ds = DataStore::getInstance();
        if(ds) ds->remove(key);

        return 0;
    }

    void DataStoreNamespace::setupNamespace(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, getInt, "getInt", 2, ".s");
        ScriptUtils::addFunction(vm, getFloat, "getFloat", 2, ".s");
        ScriptUtils::addFunction(vm, getBool, "getBool", 2, ".s");
        ScriptUtils::addFunction(vm, getString, "getString", 2, ".s");
        ScriptUtils::addFunction(vm, setValue, "setValue", 3, ".s.");
        ScriptUtils::addFunction(vm, removeValue, "removeValue", 2, ".s");
    }
}
