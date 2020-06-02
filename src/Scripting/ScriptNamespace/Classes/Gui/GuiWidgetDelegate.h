#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class GuiWidgetDelegate{
    public:
        GuiWidgetDelegate() = delete;

        static void setupWindow(HSQUIRRELVM vm);
        static void setupButton(HSQUIRRELVM vm);
        static void setupLabel(HSQUIRRELVM vm);
        static void setupEditbox(HSQUIRRELVM vm);
        static void setupSlider(HSQUIRRELVM vm);
        static void setupCheckbox(HSQUIRRELVM vm);

    private:
        static SQInteger setPosition(HSQUIRRELVM vm);
        static SQInteger setSize(HSQUIRRELVM vm);
        static SQInteger setHidden(HSQUIRRELVM vm);
        static SQInteger setText(HSQUIRRELVM vm);

        static SQInteger getText(HSQUIRRELVM vm);

        static SQInteger setSliderValue(HSQUIRRELVM vm);
        static SQInteger getSliderValue(HSQUIRRELVM vm);

        static SQInteger getCheckboxValue(HSQUIRRELVM vm);
        static SQInteger setCheckboxValue(HSQUIRRELVM vm);

        static SQInteger sizeToFit(HSQUIRRELVM vm);

        static SQInteger createButton(HSQUIRRELVM vm);
        static SQInteger createLabel(HSQUIRRELVM vm);
        static SQInteger createEditbox(HSQUIRRELVM vm);
        static SQInteger createSlider(HSQUIRRELVM vm);
        static SQInteger createCheckbox(HSQUIRRELVM vm);

        static SQInteger attachListener(HSQUIRRELVM vm);
        static SQInteger detachListener(HSQUIRRELVM vm);
    };
}