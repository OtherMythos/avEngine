#include "GlobalRegistryNamespace.h"

#include "System/BaseSingleton.h"
#include "System/Registry/ValueRegistry.h"
#include "Dialog/DialogManager.h"

namespace AV{

    SQInteger GlobalRegistryNamespace::getInt(HSQUIRRELVM vm, bool r){
        const SQChar *keyString;
        sq_getstring(vm, -1, &keyString);

        int resultVal;
        RegistryLookup result = _getRegistry(r)->getIntValue(keyString, resultVal);
        if(!lookupSuccess(result)) return 0; //Will return null in squirrel.

        sq_pushinteger(vm, resultVal);

        return 1;
    }

    SQInteger GlobalRegistryNamespace::getBool(HSQUIRRELVM vm, bool r){
        const SQChar *keyString;
        sq_getstring(vm, -1, &keyString);

        bool resultVal;
        RegistryLookup result = _getRegistry(r)->getBoolValue(keyString, resultVal);
        if(!lookupSuccess(result)) return 0;

        sq_pushbool(vm, resultVal);

        return 1;
    }

    SQInteger GlobalRegistryNamespace::getFloat(HSQUIRRELVM vm, bool r){
        const SQChar *keyString;
        sq_getstring(vm, -1, &keyString);

        float resultVal;
        RegistryLookup result = _getRegistry(r)->getFloatValue(keyString, resultVal);
        if(!lookupSuccess(result)) return 0;

        sq_pushfloat(vm, resultVal);

        return 1;
    }

    SQInteger GlobalRegistryNamespace::getString(HSQUIRRELVM vm, bool r){
        const SQChar *keyString;
        sq_getstring(vm, -1, &keyString);

        std::string resultVal;
        RegistryLookup result = _getRegistry(r)->getStringValue(keyString, resultVal);
        if(!lookupSuccess(result)) return 0;

        sq_pushstring(vm, resultVal.c_str(), -1);

        return 1;
    }

    SQInteger GlobalRegistryNamespace::setValue(HSQUIRRELVM vm, bool r){
        SQObjectType objectType = sq_gettype(vm, -1);
        if(!_isTypeAllowed(objectType)){
            sq_pushbool(vm, false);
            return 1;
        }

        const SQChar *keyString;
        sq_getstring(vm, -2, &keyString);

        auto reg = _getRegistry(r);
        switch(objectType){
            case OT_INTEGER:{
                SQInteger val;
                sq_getinteger(vm, -1, &val);
                reg->setIntValue(keyString, val);
                break;
            }
            case OT_FLOAT:{
                SQFloat val;
                sq_getfloat(vm, -1, &val);
                reg->setFloatValue(keyString, val);
                break;
            }
            case OT_BOOL:{
                SQBool val;
                sq_getbool(vm, -1, &val);
                reg->setBoolValue(keyString, val);
                break;
            }
            case OT_STRING:{
                const SQChar *val;
                sq_getstring(vm, -1, &val);
                reg->setStringValue(keyString, val);
                break;
            }
            case OT_NULL:{
                reg->removeValue(keyString);
                break;
            }
            default:{
                assert(false);
            }
        }

        sq_pushbool(vm, true);
        return 1;
    }

    SQInteger GlobalRegistryNamespace::clear(HSQUIRRELVM vm, bool r){
        _getRegistry(r)->clear();

        return 0;
    }

    SQInteger GlobalRegistryNamespace::getValue(HSQUIRRELVM vm, bool r){
        const SQChar *keyString;
        sq_getstring(vm, -1, &keyString);

        const void* v;
        RegistryType t;
        RegistryLookup result = _getRegistry(r)->getValue(keyString, v, t);
        if(!lookupSuccess(result)) return 0;

        switch(t){
            case RegistryType::STRING:{
                const std::string* s = static_cast<const std::string*>(v);
                sq_pushstring(vm, s->c_str(), -1);
                break;
            }
            case RegistryType::FLOAT:{
                const float* f = static_cast<const float*>(v);
                sq_pushfloat(vm, *f);
                break;
            }
            case RegistryType::INT:{
                const int* i = static_cast<const int*>(v);
                sq_pushinteger(vm, *i);
                break;
            }
            case RegistryType::BOOLEAN:{
                const bool* b = static_cast<const bool*>(v);
                sq_pushbool(vm, *b);
                break;
            }
            default:{
                assert(false);
            }
        }

        return 1;
    }

    bool GlobalRegistryNamespace::_isTypeAllowed(SQObjectType t){
        switch(t){
            case OT_INTEGER:
            case OT_FLOAT:
            case OT_BOOL:
            case OT_STRING:
            case OT_NULL:
                return true;
            default:
                return false;
        }
    }

    std::shared_ptr<ValueRegistry> GlobalRegistryNamespace::_getRegistry(bool registry){
        return registry ? BaseSingleton::getGlobalRegistry() : BaseSingleton::getDialogManager()->getLocalRegistry();
    }



    SQInteger GlobalRegistryNamespace::setValue(HSQUIRRELVM vm) { return setValue(vm, true); }
    SQInteger GlobalRegistryNamespace::getValue(HSQUIRRELVM vm) { return getValue(vm, true); }

    SQInteger GlobalRegistryNamespace::getInt(HSQUIRRELVM vm) { return getInt(vm, true); }
    SQInteger GlobalRegistryNamespace::getBool(HSQUIRRELVM vm) { return getBool(vm, true); }
    SQInteger GlobalRegistryNamespace::getFloat(HSQUIRRELVM vm) { return getFloat(vm, true); }
    SQInteger GlobalRegistryNamespace::getString(HSQUIRRELVM vm) { return getString(vm, true); }

    SQInteger GlobalRegistryNamespace::clear(HSQUIRRELVM vm) { return clear(vm, true); }



    void GlobalRegistryNamespace::setupNamespace(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, getInt, "getInt", 2, ".s");
        ScriptUtils::addFunction(vm, getFloat, "getFloat", 2, ".s");
        ScriptUtils::addFunction(vm, getBool, "getBool", 2, ".s");
        ScriptUtils::addFunction(vm, getString, "getString", 2, ".s");
        ScriptUtils::addFunction(vm, getValue, "get", 2, ".s");
        ScriptUtils::addFunction(vm, setValue, "set", 3, ".s.");
        ScriptUtils::addFunction(vm, clear, "clear");
    }
}