#ifdef TEST_MODE

#pragma once

#include "Scripting/ScriptNamespace/ScriptNamespace.h"

namespace AV{
    class TestModeEntityManagerNamespace : public ScriptNamespace{
    public:
        TestModeEntityManagerNamespace() {};

        virtual void setupNamespace(HSQUIRRELVM vm) {};

        void setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled);

    private:
        static SQInteger getEntityCount(HSQUIRRELVM vm);
        static SQInteger getTrackedEntityCount(HSQUIRRELVM vm);
        static SQInteger getLoadedCallbackScriptCount(HSQUIRRELVM vm);
    };
};

#endif
