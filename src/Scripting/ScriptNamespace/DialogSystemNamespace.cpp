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

    void DialogSystemNamespace::setupNamespace(HSQUIRRELVM vm){
        _addFunction(vm, unblock, "unblock");
        _addFunction(vm, compileAndRunDialog, "compileAndRunDialog");
    }

}
