#ifdef TEST_MODE

#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class TestModeGuiNamespace{
    public:
        TestModeGuiNamespace() = delete;

        static void setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled);

    private:
        static SQInteger getNumWindows(HSQUIRRELVM vm);
        static SQInteger getNumWidgets(HSQUIRRELVM vm);

        static SQInteger callListener(HSQUIRRELVM vm);
    };
};

#endif
