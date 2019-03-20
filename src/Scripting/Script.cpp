#include "Script.h"

#include <sqstdio.h>
#include <sqstdaux.h>
#include "ScriptManager.h"
#include <cstring>

#include "Event/Events/TestingEvent.h"
#include "Event/EventDispatcher.h"
#include "System/SystemSetup/SystemSettings.h"

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
            _processSquirrelFailure();
            return false;
        }

        sq_settop(vm, top);

        return true;
    }

    bool Script::runFunction(const SQChar *entry){
        if(!available || !initialised) return false;

        //The object that will contain the function (I think).
        //Object ret(vm);
        int top = sq_gettop(vm);

        sq_pushroottable(vm);

        HSQOBJECT ret;
        sq_resetobject(&ret);

        sq_pushobject(vm, obj);
        sq_pushstring(vm, entry, strlen(entry));

        if(SQ_FAILED(sq_get(vm, -3))) {
            //If a function with that name cannot be found then restore the stack to the beginning and return.
            AV_ERROR("Failed to get the function");
            sq_settop(vm, top);
            return false;
        }

        //Gets an object from the stack and stores it in the ret variable.
        //This object is the one returned from sq_get
        sq_getstackobj(vm, -1, &ret);
        //Add a reference to the new value at the top of the stack.
        //I don't really know what this does but you have to do it.
        sq_addref(vm, &ret);
        sq_pop(vm, 2);

        //Push both that function object (ret) and it's scope container (obj)
        sq_pushobject(vm, ret);
        sq_pushobject(vm, obj);

        //call and return
        if(SQ_FAILED(sq_call(vm, 1, false, false))){
            //AV_ERROR("Error in the function call.");
            _processSquirrelFailure();
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
    }

    void Script::_processSquirrelFailure(){
        const SQChar* sqErr;
        sq_getlasterror(vm);
        sq_tostring(vm, -1);
        sq_getstring(vm, -1, &sqErr);
        sq_pop(vm, 1);

        if(SystemSettings::isTestModeEnabled()){
            //If any scripts fail during a test mode run, the engine is shut down and the test is failed.
            TestingEventScriptFailure event;
            event.srcFile = filePath;
            event.failureReason = sqErr;

            EventDispatcher::transmitEvent(EventType::Testing, event);
        }else{
            AV_ERROR("There was a problem running that script file.");
            AV_ERROR(sqErr);
        }
    }
}
