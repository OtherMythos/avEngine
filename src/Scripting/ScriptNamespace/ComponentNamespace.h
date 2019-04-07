#pragma once

#include "ScriptNamespace.h"

namespace AV{
    class ComponentNamespace : public ScriptNamespace{
    public:
        ComponentNamespace() {};

        void setupNamespace(HSQUIRRELVM vm);
    };
}
