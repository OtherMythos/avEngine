#pragma once

#include "ScriptNamespace.h"

namespace AV {
    class InputNamespace : public ScriptNamespace{
    public:
        InputNamespace() {};
        
        void setupNamespace(HSQUIRRELVM vm);
        
        void setupConstants(HSQUIRRELVM vm);
        
    private:
        static SQInteger getKey(HSQUIRRELVM vm);
    };
}
