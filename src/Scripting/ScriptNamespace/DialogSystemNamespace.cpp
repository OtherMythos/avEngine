#include "DialogSystemNamespace.h"

#include "System/BaseSingleton.h"
#include "Dialog/DialogManager.h"

#include "Dialog/Compiler/DialogScriptData.h"
#include "Dialog/Compiler/DialogCompiler.h"

namespace AV{
    SQInteger DialogSystemNamespace::unblock(HSQUIRRELVM vm){
        BaseSingleton::getDialogManager()->unblock();

        return 0;
    }

    SQInteger DialogSystemNamespace::dialogReleaseHook(SQUserPointer p, SQInteger size){
        CompiledDialog* dialogPtr = static_cast<CompiledDialog*>(p);

        BaseSingleton::getDialogManager()->notifyDialogDeletion(dialogPtr);
        dialogPtr->destroy();

        return 0;
    }

    SQInteger DialogSystemNamespace::compileAndRunDialog(HSQUIRRELVM vm){
        const SQChar *path;
        sq_getstring(vm, -1, &path);

        bool result = BaseSingleton::getDialogManager()->compileAndRunDialog(path);

        sq_pushbool(vm, result);

        return 1;
    }

    SQInteger DialogSystemNamespace::compileDialog(HSQUIRRELVM vm){
        const SQChar *dialogPath;
        sq_getstring(vm, -1, &dialogPath);
        sq_pop(vm, 1);

        CompiledDialog d;
        DialogCompiler compiler;
        if(!compiler.compileScript(dialogPath, d)){
            sq_pushbool(vm, false);
            return 1;
        }


        void* pointer = (void*)sq_newuserdata(vm, sizeof(CompiledDialog));
        new (pointer)CompiledDialog();
        CompiledDialog* dialogPtr = static_cast<CompiledDialog*>(pointer);
        *dialogPtr = d;

        sq_setreleasehook(vm, -1, dialogReleaseHook);

        return 1;
    }

    SQInteger DialogSystemNamespace::executeCompiledDialog(HSQUIRRELVM vm){
        SQUserPointer pointer;
        sq_getuserdata(vm, -1, &pointer, NULL);
        CompiledDialog* dialogPtr = static_cast<CompiledDialog*>(pointer);

        BaseSingleton::getDialogManager()->beginExecution(*dialogPtr, 0);

        return 0;
    }

    SQInteger DialogSystemNamespace::isDialogExecuting(HSQUIRRELVM vm){
        SQBool result = BaseSingleton::getDialogManager()->isExecuting();
        sq_pushbool(vm, result);

        return 1;
    }

    SQInteger DialogSystemNamespace::isDialogBlocked(HSQUIRRELVM vm){
        SQBool result = BaseSingleton::getDialogManager()->isBlocked();
        sq_pushbool(vm, result);

        return 1;
    }

    void DialogSystemNamespace::setupNamespace(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, unblock, "unblock");
        ScriptUtils::addFunction(vm, compileAndRunDialog, "compileAndRunDialog");
        ScriptUtils::addFunction(vm, compileDialog, "compileDialog");
        ScriptUtils::addFunction(vm, executeCompiledDialog, "executeCompiledDialog", 2, ".d");

        ScriptUtils::addFunction(vm, isDialogExecuting, "isDialogExecuting");
        ScriptUtils::addFunction(vm, isDialogBlocked, "isDialogBlocked");
    }

}
