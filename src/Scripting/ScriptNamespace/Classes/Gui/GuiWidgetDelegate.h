#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class GuiWidgetDelegate{
    public:
        GuiWidgetDelegate() = delete;

        static void setupWindow(HSQUIRRELVM vm);
        static void setupButton(HSQUIRRELVM vm);
        static void setupLabel(HSQUIRRELVM vm);

    private:
        static SQInteger setPosition(HSQUIRRELVM vm);
        static SQInteger setSize(HSQUIRRELVM vm);
        static SQInteger setHidden(HSQUIRRELVM vm);
        static SQInteger setText(HSQUIRRELVM vm);

        static SQInteger sizeToFit(HSQUIRRELVM vm);

        static SQInteger createButton(HSQUIRRELVM vm);
        static SQInteger createLabel(HSQUIRRELVM vm);

        static SQInteger attachListener(HSQUIRRELVM vm);
    };
}
