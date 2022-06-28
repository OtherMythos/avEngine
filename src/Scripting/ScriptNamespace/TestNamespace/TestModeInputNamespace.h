#ifdef TEST_MODE

#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class TestModeInputNamespace{
    public:
        TestModeInputNamespace() = delete;

        static void setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled);

    private:
        static SQInteger sendControllerInput(HSQUIRRELVM vm);
    };
};

#endif
