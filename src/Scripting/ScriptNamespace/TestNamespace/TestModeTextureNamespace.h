#ifdef TEST_MODE

#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class TestModeTextureNamespace{
    public:
        TestModeTextureNamespace() = delete;

        static void setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled);

    private:
        static SQInteger getNumTextures(HSQUIRRELVM vm);
        static SQInteger getNumTexturesForLayer(HSQUIRRELVM vm);
        static SQInteger isTextureInLayer(HSQUIRRELVM vm);
    };
};

#endif
