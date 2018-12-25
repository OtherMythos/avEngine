#pragma once

#include <squirrel.h>

namespace AV{
    class ScriptNamespace{
    public:
        ScriptNamespace() {};

        virtual void setupNamespace(HSQUIRRELVM vm) = 0;

    protected:
        void _addFunction(HSQUIRRELVM v, SQFUNCTION f, const char *fname);
    };
}
