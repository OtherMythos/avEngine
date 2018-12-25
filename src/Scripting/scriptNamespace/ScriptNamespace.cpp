#include "ScriptNamespace.h"

namespace AV{
    void ScriptNamespace::_addFunction(HSQUIRRELVM v, SQFUNCTION f, const char *fname){
        sq_pushstring(v, _SC(fname), -1);
        sq_newclosure(v,f,0);
        sq_newslot(v,-3,SQFalse);
    }
}
