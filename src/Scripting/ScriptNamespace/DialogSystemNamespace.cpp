#include "DialogSystemNamespace.h"

#include "System/BaseSingleton.h"
#include "Dialog/DialogManager.h"

namespace AV{
    SQInteger DialogSystemNamespace::unblock(HSQUIRRELVM vm){
        BaseSingleton::getDialogManager()->unblock();

        return 0;
    }

    SQInteger DialogSystemNamespace::compileAndRunDialog(HSQUIRRELVM vm){
        const SQChar *path;
        sq_getstring(vm, -1, &path);

        bool result = BaseSingleton::getDialogManager()->compileAndRunDialog(path);

        sq_pushbool(vm, result);

        return 1;
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

        ScriptUtils::addFunction(vm, isDialogExecuting, "isDialogExecuting");
        ScriptUtils::addFunction(vm, isDialogBlocked, "isDialogBlocked");
    }

}
