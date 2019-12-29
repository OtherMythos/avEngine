#include "DialogScriptImplementation.h"

#include "System/SystemSetup/SystemSettings.h"
#include "Scripting/ScriptManager.h"
#include "Scripting/Script/CallbackScript.h"

namespace AV{
    DialogScriptImplementation::DialogScriptImplementation(){

    }

    DialogScriptImplementation::~DialogScriptImplementation(){

    }

    void DialogScriptImplementation::initialise(){
        if(!SystemSettings::isDialogImplementationScriptViable()) return;

        mScript = new CallbackScript();
        ScriptManager::initialiseCallbackScript(mScript);

        const std::string& scriptPath = SystemSettings::getDialogImplementationScriptPath();
        mScript->prepare(scriptPath.c_str());
        mSetupCorrectly = true;

        FIDdialogString = mScript->getCallbackId("dialogString");
        FIDdialogBegin = mScript->getCallbackId("dialogBegan");
        FIDdialogEnded = mScript->getCallbackId("dialogEnded");

    }

    static const std::string* strPtr = 0;
    SQInteger dialogStringPopulate(HSQUIRRELVM vm){
        assert(strPtr);
        sq_pushstring(vm, strPtr->c_str(), -1);
        sq_pushinteger(vm, 1);

        strPtr = 0;
        return 3;
    }

    void DialogScriptImplementation::notifyDialogString(const std::string& str){
        strPtr = &str;
        mScript->call(FIDdialogString, dialogStringPopulate);
    }

    void DialogScriptImplementation::notifyDialogExecutionBegin(){
        mScript->call(FIDdialogBegin);
    }

    void DialogScriptImplementation::notifyDialogExecutionEnded(){
        mScript->call(FIDdialogEnded);
    }
}
