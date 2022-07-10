#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class GuiSizerDelegate{
    public:
        GuiSizerDelegate() = delete;

        static void setupLayoutLine(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

    private:
        static SQInteger layout(HSQUIRRELVM vm);
        static SQInteger addCell(HSQUIRRELVM vm);
        static SQInteger setCellOffset(HSQUIRRELVM vm);
        static SQInteger setMarginForAllCells(HSQUIRRELVM vm);
        static SQInteger setPosition(HSQUIRRELVM vm);
        static SQInteger setGridLocationForAllCells(HSQUIRRELVM vm);
    };
}
