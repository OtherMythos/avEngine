#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "ColibriGui/ColibriGuiPrerequisites.h"

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
        static void setupSpinner(HSQUIRRELVM vm);
        static void setupAnimatedLabel(HSQUIRRELVM vm);

    private:
        static SQInteger getPosition(HSQUIRRELVM vm);
        static SQInteger getDerivedPosition(HSQUIRRELVM vm);
        static SQInteger getDerivedCentre(HSQUIRRELVM vm);
        static SQInteger getCentre(HSQUIRRELVM vm);
        static SQInteger getSize(HSQUIRRELVM vm);
        static SQInteger getSizeAfterClipping(HSQUIRRELVM vm);
        static SQInteger calculateChildrenSize(HSQUIRRELVM vm);

        static SQInteger setPosition(HSQUIRRELVM vm);
        static SQInteger setCentre(HSQUIRRELVM vm);
        static SQInteger setSize(HSQUIRRELVM vm);
        static SQInteger setHidden(HSQUIRRELVM vm);
        static SQInteger setVisible(HSQUIRRELVM vm);
        static SQInteger getVisible(HSQUIRRELVM vm);
        static SQInteger setText(HSQUIRRELVM vm);
        static SQInteger setZOrder(HSQUIRRELVM vm);
        static SQInteger getZOrder(HSQUIRRELVM vm);
        static SQInteger setAnimatedGlyph(HSQUIRRELVM vm);
        static SQInteger getType(HSQUIRRELVM vm);

        static SQInteger getNumChildren(HSQUIRRELVM vm);
        static SQInteger getChildForIdx(HSQUIRRELVM vm);

        static SQInteger setSpinnerOptions(HSQUIRRELVM vm);
        static SQInteger getSpinnerValue(HSQUIRRELVM vm);
        static SQInteger getSpinnerValueRaw(HSQUIRRELVM vm);
        static SQInteger setSpinnerValueRaw(HSQUIRRELVM vm);
        static SQInteger setVisualsEnabled(HSQUIRRELVM vm);

        static SQInteger getText(HSQUIRRELVM vm);
        static SQInteger setDefaultFont(HSQUIRRELVM vm);
        static SQInteger setDefaultFontSize(HSQUIRRELVM vm);
        static SQInteger getDefaultFontSize(HSQUIRRELVM vm);
        static SQInteger setTextColour(HSQUIRRELVM vm);
        static SQInteger setTextHorizontalAlignment(HSQUIRRELVM vm);
        static SQInteger setRichText(HSQUIRRELVM vm);
        static SQInteger setShadowOutline(HSQUIRRELVM vm);
        static SQInteger setNextWidget(HSQUIRRELVM vm);

        static SQInteger setSliderValue(HSQUIRRELVM vm);
        static SQInteger getSliderValue(HSQUIRRELVM vm);
        static SQInteger setSliderRange(HSQUIRRELVM vm);

        static SQInteger getCheckboxValue(HSQUIRRELVM vm);
        static SQInteger setCheckboxValue(HSQUIRRELVM vm);

        static SQInteger getWidgetUserId(HSQUIRRELVM vm);
        static SQInteger setWidgetUserId(HSQUIRRELVM vm);

        static SQInteger sizeToFit(HSQUIRRELVM vm);
        static SQInteger sizeToFitLabel(HSQUIRRELVM vm);

        static SQInteger createButton(HSQUIRRELVM vm);
        static SQInteger createLabel(HSQUIRRELVM vm);
        static SQInteger createAnimatedLabel(HSQUIRRELVM vm);
        static SQInteger createEditbox(HSQUIRRELVM vm);
        static SQInteger createSlider(HSQUIRRELVM vm);
        static SQInteger createCheckbox(HSQUIRRELVM vm);
        static SQInteger createPanel(HSQUIRRELVM vm);
        static SQInteger createSpinner(HSQUIRRELVM vm);
        static SQInteger createWindow(HSQUIRRELVM vm);

        static SQInteger windowGetCurrentScroll(HSQUIRRELVM vm);
        static SQInteger windowSizeScrollToFit(HSQUIRRELVM vm);
        static SQInteger windowSetMaxScroll(HSQUIRRELVM vm);
        static SQInteger windowSetAllowMouseScroll(HSQUIRRELVM vm);
        static SQInteger windowSetConsumeCursor(HSQUIRRELVM vm);
        static SQInteger windowSetColour(HSQUIRRELVM vm);
        static SQInteger windowGetQueryName(HSQUIRRELVM vm);

        static SQInteger setClickable(HSQUIRRELVM vm);
        static SQInteger setKeyboardNavigable(HSQUIRRELVM vm);
        static SQInteger setFocus(HSQUIRRELVM vm);

        static SQInteger attachListener(HSQUIRRELVM vm);
        static SQInteger attachListenerForEvent(HSQUIRRELVM vm);
        static SQInteger detachListener(HSQUIRRELVM vm);

        static SQInteger setSkin(HSQUIRRELVM vm);
        static SQInteger setSkinPack(HSQUIRRELVM vm);

        static SQInteger setOrientation(HSQUIRRELVM vm);
        static SQInteger setClipBorders(HSQUIRRELVM vm);

        static SQInteger setDatablock(HSQUIRRELVM vm);
        static SQInteger getDatablock(HSQUIRRELVM vm);

        static SQInteger setExpandVertical(HSQUIRRELVM vm);
        static SQInteger setExpandHorizontal(HSQUIRRELVM vm);
        static SQInteger setProportionVertical(HSQUIRRELVM vm);
        static SQInteger setProportionHorizontal(HSQUIRRELVM vm);
        static SQInteger setPriority(HSQUIRRELVM vm);
        static SQInteger setMargin(HSQUIRRELVM vm);
        static SQInteger setMinSize(HSQUIRRELVM vm);
        static SQInteger setGridLocation(HSQUIRRELVM vm);
        static SQInteger setDisabled(HSQUIRRELVM vm);
        static SQInteger setBreadthFirst(HSQUIRRELVM vm);
    };
}
