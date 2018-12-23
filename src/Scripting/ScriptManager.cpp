#include "ScriptManager.h"
#include "Logger/Log.h"

#include <iostream>

#include <sqstdio.h>

#ifdef SQUNICODE
#define scvprintf vwprintf
#else
#define scvprintf vprintf
#endif

namespace AV {
    HSQUIRRELVM ScriptManager::_sqvm = sq_open(1024);
    
    void printfunc(HSQUIRRELVM v, const SQChar *s, ...){
        va_list arglist;
        va_start(arglist, s);
        scvprintf(s, arglist);
        va_end(arglist);
        std::cout << '\n';
    }
    
    void ScriptManager::initialise(){
        _setupVM(_sqvm);
    }
    
    void ScriptManager::runScript(const char *scriptPath){
        AV_INFO("Running Script {}", scriptPath);
        sq_pushroottable(_sqvm);
        if(SQ_SUCCEEDED(sqstd_dofile(_sqvm, _SC(scriptPath), 0, 1))){
            AV_INFO("Succeeded");
        }else{
            AV_ERROR("There was a problem loading that script file.");
        }
    }
    
    void ScriptManager::_setupVM(HSQUIRRELVM vm){
        sq_setprintfunc(vm, printfunc, NULL);
        
        sq_pushroottable(vm);
        

        
        sq_pop(vm,1);
    }
}
