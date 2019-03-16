#include "ScriptNamespace.h"

namespace AV{
    void ScriptNamespace::_addFunction(HSQUIRRELVM v, SQFUNCTION f, const char *fname, int numParams, const char *typeMask){
        sq_pushstring(v, _SC(fname), -1);
        sq_newclosure(v,f,0);
        if(numParams != 0){
            sq_setparamscheck(v,numParams,_SC(typeMask));
        }
        sq_newslot(v,-3,SQFalse);
    }

    void ScriptNamespace::_redirectFunctionMap(HSQUIRRELVM v, SQFUNCTION redirectFunction, const RedirectFunctionMap &rMap, bool redirect){
        auto it = rMap.begin();
        while(it != rMap.end()){
            if(redirect){
                const TestFunctionEntry& entry = (*it).second;
                _addFunction(v, entry.function, (*it).first, entry.typeCount, entry.typeMask);
            }else{
                _addFunction(v, redirectFunction, (*it).first);
            }

            it++;
        }
    }

    SQInteger ScriptNamespace::EIDReleaseHook(SQUserPointer p, SQInteger size){
        squirrelEIdData **data = static_cast<squirrelEIdData**>(p);

        delete *data;

        return 0;
    }

    void ScriptNamespace::_entityClassFromEID(HSQUIRRELVM vm, eId entity){
        sq_pushroottable(vm);
        sq_pushstring(vm, _SC("entity"), 6);
        sq_rawget(vm, -2);

        sq_createinstance(vm, -1);

        eId* instanceId = new eId(entity);
        sq_setinstanceup(vm, -1, (SQUserPointer*)instanceId);

        //TODO add a release hook to the instance.


        // squirrelEIdData** ud = reinterpret_cast<squirrelEIdData**>(sq_newuserdata(vm, sizeof (squirrelEIdData*)));
        // *ud = new squirrelEIdData(entity);
        //
        // sq_setreleasehook(vm, -1, EIDReleaseHook);
    }

}
