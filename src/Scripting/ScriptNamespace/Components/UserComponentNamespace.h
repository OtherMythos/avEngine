#pragma once

#include "squirrel.h"
#include "System/EnginePrerequisites.h"

namespace AV{
    class UserComponentNamespace{
    public:
        UserComponentNamespace() {}

        void setupNamespace(HSQUIRRELVM vm);

        template <uint8 A>
        static SQInteger add(HSQUIRRELVM vm);
        template <uint8 A>
        static SQInteger remove(HSQUIRRELVM v);
        template <uint8 A>
        static SQInteger set(HSQUIRRELVM vm);

    private:
        static SQInteger _add(HSQUIRRELVM v, uint8 i);
        static SQInteger _remove(HSQUIRRELVM vm, uint8 i);
        static SQInteger _set(HSQUIRRELVM vm, uint8 i);

        static SQInteger userComponentGetMetamethod(HSQUIRRELVM vm);
    };
}

