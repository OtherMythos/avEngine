#pragma once

#include "ScriptNamespace.h"

namespace AV{
    class ScriptManager;
    class WorldNamespace : public ScriptNamespace{
        friend ScriptManager;
    public:
        WorldNamespace() {};

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger createWorld(HSQUIRRELVM vm);
        static SQInteger destroyWorld(HSQUIRRELVM vm);
    };
}
