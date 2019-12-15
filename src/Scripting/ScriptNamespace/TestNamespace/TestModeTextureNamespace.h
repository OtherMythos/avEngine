#ifdef TEST_MODE

#pragma once

#include "Scripting/ScriptNamespace/ScriptNamespace.h"

namespace AV{
    class TestModeTextureNamespace : public ScriptNamespace{
    public:
        TestModeTextureNamespace() {};

        virtual void setupNamespace(HSQUIRRELVM vm) {};

        virtual void setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled);

    private:
        static SQInteger getNumTextures(HSQUIRRELVM vm);
        static SQInteger getNumTexturesForLayer(HSQUIRRELVM vm);
        static SQInteger isTextureInLayer(HSQUIRRELVM vm);
    };
};

#endif
