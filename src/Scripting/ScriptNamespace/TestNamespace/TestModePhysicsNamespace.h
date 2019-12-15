#ifdef TEST_MODE

#pragma once

#include "Scripting/ScriptNamespace/ScriptNamespace.h"

namespace AV{
    class TestModePhysicsNamespace : public ScriptNamespace{
    public:
        TestModePhysicsNamespace() {};

        virtual void setupNamespace(HSQUIRRELVM vm) {};

        virtual void setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled);

    private:
        static SQInteger getShapeExists(HSQUIRRELVM vm);
    };
};

#endif
