#pragma once

#include "ScriptUtils.h"

namespace AV{
    class DialogSystemNamespace{
    public:
        DialogSystemNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger unblock(HSQUIRRELVM vm);
        static SQInteger compileAndRunDialog(HSQUIRRELVM vm);
        static SQInteger isDialogExecuting(HSQUIRRELVM vm);
        static SQInteger isDialogBlocked(HSQUIRRELVM vm);
    };
}
