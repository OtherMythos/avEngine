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
    
    void ScriptNamespace::_declareConstant(HSQUIRRELVM vm, const char* name, SQInteger val){
        sq_pushstring(vm, _SC(name), -1);
        sq_pushinteger(vm, val);
        sq_newslot(vm, -3 , false);
    }

}
