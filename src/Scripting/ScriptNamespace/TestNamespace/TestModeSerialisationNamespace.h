#ifdef TEST_MODE

#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class TestModeSerialisationNamespace{
    public:
        TestModeSerialisationNamespace() = delete;

        static void setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled);

    private:
        static SQInteger assureSaveDirectory(HSQUIRRELVM vm);
    };
};

#endif
