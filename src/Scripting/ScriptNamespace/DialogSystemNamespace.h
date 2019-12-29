#pragma once

#include "ScriptNamespace.h"

namespace AV{
    class DialogSystemNamespace : public ScriptNamespace{
    public:
        DialogSystemNamespace() {};

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger unblock(HSQUIRRELVM vm);
        static SQInteger compileAndRunDialog(HSQUIRRELVM vm);
    };
}
