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
        static void setupPanel(HSQUIRRELVM vm);

    private:
        static SQInteger setPosition(HSQUIRRELVM vm);
        static SQInteger setSize(HSQUIRRELVM vm);
        static SQInteger setHidden(HSQUIRRELVM vm);
        static SQInteger setText(HSQUIRRELVM vm);
        static SQInteger setZOrder(HSQUIRRELVM vm);

        static SQInteger getText(HSQUIRRELVM vm);
        static SQInteger setDefaultFont(HSQUIRRELVM vm);
        static SQInteger setDefaultFontSize(HSQUIRRELVM vm);
        static SQInteger setTextColour(HSQUIRRELVM vm);
        static SQInteger setTextHorizontalAlignment(HSQUIRRELVM vm);
        static SQInteger setRichText(HSQUIRRELVM vm);

        static SQInteger setSliderValue(HSQUIRRELVM vm);
        static SQInteger getSliderValue(HSQUIRRELVM vm);

        static SQInteger getCheckboxValue(HSQUIRRELVM vm);
        static SQInteger setCheckboxValue(HSQUIRRELVM vm);

        static SQInteger getWidgetUserId(HSQUIRRELVM vm);
        static SQInteger setWidgetUserId(HSQUIRRELVM vm);

        static SQInteger sizeToFit(HSQUIRRELVM vm);

        static SQInteger createButton(HSQUIRRELVM vm);
        static SQInteger createLabel(HSQUIRRELVM vm);
        static SQInteger createEditbox(HSQUIRRELVM vm);
        static SQInteger createSlider(HSQUIRRELVM vm);
        static SQInteger createCheckbox(HSQUIRRELVM vm);
        static SQInteger createPanel(HSQUIRRELVM vm);
        static SQInteger createWindow(HSQUIRRELVM vm);

        static SQInteger setClickable(HSQUIRRELVM vm);
        static SQInteger setKeyboardNavigable(HSQUIRRELVM vm);
        static SQInteger setFocus(HSQUIRRELVM vm);

        static SQInteger attachListener(HSQUIRRELVM vm);
        static SQInteger detachListener(HSQUIRRELVM vm);

        static SQInteger setSkin(HSQUIRRELVM vm);
        static SQInteger setSkinPack(HSQUIRRELVM vm);

        static SQInteger setOrientation(HSQUIRRELVM vm);

        static SQInteger setDatablock(HSQUIRRELVM vm);
        static SQInteger getDatablock(HSQUIRRELVM vm);
    };
}
