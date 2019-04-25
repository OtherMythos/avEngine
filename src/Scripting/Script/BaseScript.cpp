#include "BaseScript.h"

#include "Event/Events/TestingEvent.h"
#include "Event/EventDispatcher.h"
#include "System/SystemSetup/SystemSettings.h"

#include "Logger/Log.h"

namespace AV{
    BaseScript::BaseScript(){

    }

    BaseScript::~BaseScript(){

    }

    void BaseScript::_processSquirrelFailure(HSQUIRRELVM vm){
        const SQChar* sqErr;
        sq_getlasterror(vm);
        sq_tostring(vm, -1);
        sq_getstring(vm, -1, &sqErr);
        sq_pop(vm, 1);
        
        //For some reason this doesn't work.
        //My theory is that the stack isn't active (the script has failed) by the time this occurs.
        //TODO figure this out because line numbers would be helpful here.
        SQStackInfos si;
        sq_stackinfos(vm, 1, &si);

        if(SystemSettings::isTestModeEnabled()){
            //If any scripts fail during a test mode run, the engine is shut down and the test is failed.
            TestingEventScriptFailure event;
            event.srcFile = filePath;
            event.failureReason = sqErr;
            //event.functionName = si.funcname;
            //event.lineNum = si.line;

            EventDispatcher::transmitEvent(EventType::Testing, event);
        }else{
            AV_ERROR("There was a problem running that script file.");
            AV_ERROR(sqErr);
            //AV_ERROR("In line {} of function {}.", si.line, si.funcname);
        }
    }
}
