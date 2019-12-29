#include "DialogSystemNamespace.h"

#include "System/BaseSingleton.h"
#include "Dialog/DialogManager.h"

namespace AV{
    SQInteger DialogSystemNamespace::unblock(HSQUIRRELVM vm){
        BaseSingleton::getDialogManager()->unblock();

        return 0;
    }

    void DialogSystemNamespace::setupNamespace(HSQUIRRELVM vm){
        _addFunction(vm, unblock, "unblock");
    }

}
