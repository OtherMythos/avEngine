#pragma once

#include "Scripting/ScriptNamespace/ScriptNamespace.h"

namespace AV{
    class ScriptManager;
    class TestNamespace : public ScriptNamespace{
        friend ScriptManager;
    public:
        TestNamespace() {};

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger assertTrue(HSQUIRRELVM vm);
        static SQInteger testModeDisabledMessage(HSQUIRRELVM vm);
    };
}
