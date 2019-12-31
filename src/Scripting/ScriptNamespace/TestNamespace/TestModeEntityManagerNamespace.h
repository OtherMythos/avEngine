#ifdef TEST_MODE

#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class TestModeEntityManagerNamespace{
    public:
        TestModeEntityManagerNamespace() = delete;

        static void setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled);

    private:
        static SQInteger getEntityCount(HSQUIRRELVM vm);
        static SQInteger getTrackedEntityCount(HSQUIRRELVM vm);
        static SQInteger getLoadedCallbackScriptCount(HSQUIRRELVM vm);
    };
};

#endif
