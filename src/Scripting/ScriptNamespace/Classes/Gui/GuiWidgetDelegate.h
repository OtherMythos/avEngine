#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class GuiWidgetDelegate{
    public:
        GuiWidgetDelegate() = delete;

        static void setupTable(HSQUIRRELVM vm);

    private:
        static SQInteger setPosition(HSQUIRRELVM vm);
        static SQInteger setSize(HSQUIRRELVM vm);
        static SQInteger setHidden(HSQUIRRELVM vm);
    };
}
