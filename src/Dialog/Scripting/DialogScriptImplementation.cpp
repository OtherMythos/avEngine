#include "DialogScriptImplementation.h"

#include "System/SystemSetup/SystemSettings.h"
#include "Scripting/ScriptVM.h"
#include "Scripting/Script/CallbackScript.h"
#include "Scripting/ScriptManager.h"
#include "Dialog/Compiler/DialogScriptData.h"

#include "System/BaseSingleton.h"

namespace AV{
    DialogScriptImplementation::DialogScriptImplementation(){

    }

    DialogScriptImplementation::~DialogScriptImplementation(){

    }

    void DialogScriptImplementation::initialise(){
        if(!SystemSettings::isDialogImplementationScriptViable()) return;

        const std::string& scriptPath = SystemSettings::getDialogImplementationScriptPath();
        mScript = BaseSingleton::getScriptManager()->loadScript(scriptPath);

        mSetupCorrectly = true;

        FIDdialogString = mScript->getCallbackId("dialogString");
        FIDdialogBegin = mScript->getCallbackId("dialogBegan");
        FIDdialogEnded = mScript->getCallbackId("dialogEnded");
        FIDdialogOption = mScript->getCallbackId("dialogOption");
        FIDactorMoveTo = mScript->getCallbackId("actorMoveTo");
        FIDactorChangeDirection = mScript->getCallbackId("actorChangeDirection");
        FIDHideDialogWindow = mScript->getCallbackId("hideDialogWindow");
        FIDShouldCloseDialogWindow = mScript->getCallbackId("shouldCloseDialogWindow");

    }

    static const std::string* strPtr = 0;
    SQInteger dialogStringPopulate(HSQUIRRELVM vm){
        assert(strPtr);
        sq_pushstring(vm, strPtr->c_str(), -1);
        sq_pushinteger(vm, 1);

        strPtr = 0;
        return 3;
    }

    static const std::string *dialogOptionStrings[MAX_DIALOG_OPTIONS];
    SQInteger dialogOptionPopulate(HSQUIRRELVM vm){
        sq_newarray(vm, 0);
        for(uint8 i = 0; i < MAX_DIALOG_OPTIONS; i++){
            if(!dialogOptionStrings[i]) break;
            sq_pushstring(vm, dialogOptionStrings[i]->c_str(), -1);
            sq_arrayinsert(vm, -2, i);
        }

        memset(&dialogOptionStrings, 0, sizeof(dialogOptionStrings));
        return 2;
    }

    static const Entry4* e4Ptr = 0;
    SQInteger e4Populate(HSQUIRRELVM vm){
        assert(e4Ptr);
        sq_pushinteger(vm, e4Ptr->w);
        sq_pushinteger(vm, e4Ptr->x);
        sq_pushinteger(vm, e4Ptr->y);
        sq_pushinteger(vm, e4Ptr->z);

        e4Ptr = 0;
        return 5;
    }

    static const Entry2* e2Ptr = 0;
    SQInteger e2Populate(HSQUIRRELVM vm){
        assert(e2Ptr);
        sq_pushinteger(vm, e2Ptr->x);
        sq_pushinteger(vm, e2Ptr->y);

        e2Ptr = 0;
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

    void DialogScriptImplementation::notifyActorMoveTo(const Entry4& e){
        e4Ptr = &e;
        mScript->call(FIDactorMoveTo, e4Populate);
    }

    void DialogScriptImplementation::notifyActorChangeDirection(const Entry2& e){
        e2Ptr = &e;
        mScript->call(FIDactorChangeDirection, e2Populate);
    }

    void DialogScriptImplementation::hideDialogWindow(){
        mScript->call(FIDHideDialogWindow);
    }

    void DialogScriptImplementation::notifyShouldCloseDialog(){
        mScript->call(FIDShouldCloseDialogWindow);
    }

    void DialogScriptImplementation::notifyOption(const std::string** strings){
        for(uint8 i = 0; i < MAX_DIALOG_OPTIONS; i++){
            dialogOptionStrings[i] = *strings++;
        }
        mScript->call(FIDdialogOption, dialogOptionPopulate);
    }
}
