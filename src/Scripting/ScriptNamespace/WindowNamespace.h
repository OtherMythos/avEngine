#pragma once

#include "ScriptNamespace.h"

namespace AV{
    class ScriptManager;

    class WindowNamespace : public ScriptNamespace{
        friend ScriptManager;
    public:
        WindowNamespace() {};

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger getWidth(HSQUIRRELVM vm);
        static SQInteger getHeight(HSQUIRRELVM vm);
    };
}
