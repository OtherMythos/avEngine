#pragma once

#include "Scripting/ScriptNamespace/ScriptNamespace.h"

namespace AV{
    class TestModeSerialisationNamespace : public ScriptNamespace{
    public:
        TestModeSerialisationNamespace() {};

        virtual void setupNamespace(HSQUIRRELVM vm) {};

        virtual void setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled);

    private:
        static SQInteger assureSaveDirectory(HSQUIRRELVM vm);
    };
};
