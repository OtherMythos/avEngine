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
