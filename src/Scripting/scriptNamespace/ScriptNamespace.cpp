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
}
