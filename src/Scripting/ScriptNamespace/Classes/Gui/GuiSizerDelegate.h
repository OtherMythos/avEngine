#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class GuiSizerDelegate{
    public:
        GuiSizerDelegate() = delete;

        static void setupLayoutLine(HSQUIRRELVM vm);
        static void setupLayoutTable(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

    private:
        static SQInteger layout(HSQUIRRELVM vm);
        static SQInteger addCell(HSQUIRRELVM vm);
        static SQInteger setCellOffset(HSQUIRRELVM vm);
        static SQInteger setMarginForAllCells(HSQUIRRELVM vm);
        static SQInteger setPosition(HSQUIRRELVM vm);
        static SQInteger setGridLocationForAllCells(HSQUIRRELVM vm);
        static SQInteger setHardMaxSize(HSQUIRRELVM vm);
        static SQInteger getNumCells(HSQUIRRELVM vm);
        static SQInteger setCellExpandVertical(HSQUIRRELVM vm);
        static SQInteger setCellExpandHorizontal(HSQUIRRELVM vm);
        static SQInteger setCellPriority(HSQUIRRELVM vm);
        static SQInteger setCellProportionVertical(HSQUIRRELVM vm);
        static SQInteger setCellProportionHorizontal(HSQUIRRELVM vm);
        static SQInteger setCellMinSize(HSQUIRRELVM vm);
        static SQInteger setCellSize(HSQUIRRELVM vm);
    };
}
