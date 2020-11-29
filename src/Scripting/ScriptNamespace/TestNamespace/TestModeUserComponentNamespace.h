#ifdef TEST_MODE

#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class TestModeUserComponentNamespace{
    public:
        TestModeUserComponentNamespace() = delete;

        static void setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled);

    private:
        static SQInteger getNumUserComponents(HSQUIRRELVM vm);
        static SQInteger getUserComponentNames(HSQUIRRELVM vm);
    };
};

#endif
