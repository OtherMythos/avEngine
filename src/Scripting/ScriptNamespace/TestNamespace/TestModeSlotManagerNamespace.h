#pragma once

#include "Scripting/ScriptNamespace/ScriptNamespace.h"

namespace AV{
    class TestModeSlotManagerNamespace : public ScriptNamespace{
    public:
        TestModeSlotManagerNamespace() {};

        virtual void setupNamespace(HSQUIRRELVM vm) {};

        void setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled);

    private:
        static SQInteger getQueueSize(HSQUIRRELVM vm);
    };
};
