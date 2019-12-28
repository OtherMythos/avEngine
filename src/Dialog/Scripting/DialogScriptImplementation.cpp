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

    }
}
