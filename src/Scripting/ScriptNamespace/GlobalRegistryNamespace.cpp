#include "GlobalRegistryNamespace.h"

#include "System/BaseSingleton.h"
#include "System/Registry/ValueRegistry.h"

namespace AV{

    SQInteger GlobalRegistryNamespace::getInt(HSQUIRRELVM vm){
        const SQChar *keyString;
        sq_getstring(vm, -1, &keyString);

        int resultVal;
        RegistryLookup result = BaseSingleton::getGlobalRegistry()->getIntValue(keyString, resultVal);
        if(!lookupSuccess(result)) return 0; //Will return null in squirrel.

        sq_pushinteger(vm, resultVal);

        return 1;
    }

    SQInteger GlobalRegistryNamespace::setValue(HSQUIRRELVM vm){
        SQObjectType objectType = sq_gettype(vm, -1);
        if(!_isTypeAllowed(objectType)){
            sq_pushbool(vm, false);
            return 1;
        }

        const SQChar *keyString;
        sq_getstring(vm, -2, &keyString);

        auto reg = BaseSingleton::getGlobalRegistry();
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
            case OT_STRING:
            default:{
                assert(false);
            }
        }

        sq_pushbool(vm, true);
        return 1;
    }

    bool GlobalRegistryNamespace::_isTypeAllowed(SQObjectType t){
        switch(t){
            case OT_INTEGER:
            case OT_FLOAT:
            case OT_BOOL:
            case OT_STRING:
                return true;
            default:
                return false;
        }
    }

    void GlobalRegistryNamespace::setupNamespace(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, getInt, "getInt", 2, ".s");
        ScriptUtils::addFunction(vm, setValue, "set", 3, ".s.");
    }
}
