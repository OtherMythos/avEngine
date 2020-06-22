#include "Script.h"

#include <sqstdio.h>
#include <sqstdaux.h>
#include "Scripting/ScriptVM.h"
#include <cstring>

#include "Logger/Log.h"

namespace AV{
    Script::Script(HSQUIRRELVM vm)
        : vm(vm),
        initialised(true){
            sq_resetobject(&obj);
    }

    Script::Script(){
        sq_resetobject(&obj);
    }

    Script::~Script(){
        release();
    }

    void Script::initialise(HSQUIRRELVM vm){
        this->vm = vm;
        initialised = true;
    }

    bool Script::compileFile(const SQChar *path){
        if(!initialised) return false;

        sq_release(vm, &obj);
        sq_resetobject(&obj);

        if(SQ_FAILED(sqstd_loadfile(vm, path, true))){
            AV_ERROR("loading file failed");
            return false;
        }

        filePath = std::string(path);

        sq_getstackobj(vm,-1,&obj);
        sq_addref(vm, &obj);
        sq_pop(vm, 1);

        available = true;

        return true;
    }

    bool Script::run(){
        if(!available || !initialised) return false;

        SQInteger top = sq_gettop(vm);
        sq_pushobject(vm, obj);
        sq_pushroottable(vm);
        if(SQ_FAILED(sq_call(vm, 1, false, true))){
            _processSquirrelFailure(vm);
            return false;
        }

        sq_settop(vm, top);

        return true;
    }

    void Script::release(){
        if(!sq_isnull(obj)) {
            sq_resetobject(&obj);
            sq_release(vm, &obj);
        }
        available = false;
        filePath = "";
    }
}
