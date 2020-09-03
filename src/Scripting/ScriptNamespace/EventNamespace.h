#pragma once

#include "ScriptUtils.h"

namespace AV{
    class ScriptVM;
    class EventNamespace{
        friend ScriptVM;
    public:
        EventNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger subscribe(HSQUIRRELVM vm);
    };
}
