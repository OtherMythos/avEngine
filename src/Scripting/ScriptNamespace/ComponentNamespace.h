#pragma once

#include "ScriptUtils.h"

namespace AV{
    class ComponentNamespace{
    public:
        ComponentNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);
    };
}
