#pragma once

#include "squirrel.h"

namespace AV{
    class ScriptComponentNamespace{
    public:
        ScriptComponentNamespace() {}
        
        void setupNamespace(HSQUIRRELVM vm);
        
    private:
        static SQInteger add(HSQUIRRELVM v);
        static SQInteger remove(HSQUIRRELVM v);
    };
}

