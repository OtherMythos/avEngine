#ifdef TEST_MODE

#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include <string>

namespace AV{
    class TestNamespace{
    public:
        TestNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:

        static SQInteger assertTrue(HSQUIRRELVM vm);
        static SQInteger assertFalse(HSQUIRRELVM vm);
        static SQInteger assertEqual(HSQUIRRELVM vm);
        static SQInteger assertNotEqual(HSQUIRRELVM vm);
        static SQInteger testModeDisabledMessage(HSQUIRRELVM vm);
        static SQInteger endTest(HSQUIRRELVM vm);

        static std::string _getTypeString(SQObjectType type);
        static SQInteger _processBooleanAssert(HSQUIRRELVM vm, bool intendedResult);
        static SQInteger _processComparisonAssert(HSQUIRRELVM vm, bool equalsComparison);
        static std::string _obtainSourceCodeLine(const std::string& path, int lineNumber);
    };
}

#endif
