#include "GuiSizerDelegate.h"

#include "Scripting/ScriptNamespace/GuiNamespace.h"

#include "ColibriGui/ColibriWidget.h"
#include "ColibriGui/ColibriButton.h"
#include "ColibriGui/ColibriLabel.h"
#include "ColibriGui/Layouts/ColibriLayoutLine.h"

#include "Scripting/ScriptObjectTypeTags.h"

namespace AV{

    void GuiSizerDelegate::setupLayoutLine(HSQUIRRELVM vm){
        sq_newtableex(vm, 1);

        ScriptUtils::addFunction(vm, layout, "layout");
        ScriptUtils::addFunction(vm, addCell, "addCell", 2, ".u");
    }

    SQInteger GuiSizerDelegate::addCell(HSQUIRRELVM vm){
        Colibri::LayoutBase* layout = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getLayoutFromUserData(vm, -2, &layout));

        Colibri::LayoutCell* target = 0;

        SQUserPointer typeTag;
        sq_gettypetag(vm, -1, &typeTag);
        if(typeTag == LayoutLineTypeTag){
            Colibri::LayoutBase* layout = 0;
            SCRIPT_CHECK_RESULT(GuiNamespace::getLayoutFromUserData(vm, -1, &layout));
            target = dynamic_cast<Colibri::LayoutCell*>(layout);

            if(layout == target) return sq_throwerror(vm, "Attempting to add a layout to itself");
        }else{
            Colibri::Widget* widget = 0;
            void* foundType = 0;
            SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, -1, &widget, &foundType));
            if(!GuiNamespace::isTypeTagBasicWidget(foundType)) return 0;
            target = dynamic_cast<Colibri::LayoutCell*>(widget);
        }

        ((Colibri::LayoutLine*)layout)->addCell(target);

        return 0;
    }

    SQInteger GuiSizerDelegate::layout(HSQUIRRELVM vm){
        Colibri::LayoutBase* layout = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getLayoutFromUserData(vm, -1, &layout));

        ((Colibri::LayoutLine*)layout)->layout();

        return 0;
    }

    void GuiSizerDelegate::setupConstants(HSQUIRRELVM vm){
        /**SQNamespace
        @name _gui
        */
        /**SQConstant
        @name _LAYOUT_VERTICAL
        */
        ScriptUtils::declareConstant(vm, "_LAYOUT_VERTICAL", 0);
        /**SQConstant
        @name _LAYOUT_HORIZONTAL
        */
        ScriptUtils::declareConstant(vm, "_LAYOUT_HORIZONTAL", 1);
        /**SQConstant
        @name _TEXT_ALIGN_NATURAL
        @desc Align text according to the specified global alignment.
        */
        ScriptUtils::declareConstant(vm, "_TEXT_ALIGN_NATURAL", 0);
        /**SQConstant
        @name _TEXT_ALIGN_LEFT
        @desc Align text to the left.
        */
        ScriptUtils::declareConstant(vm, "_TEXT_ALIGN_LEFT", 1);
        /**SQConstant
        @name _TEXT_ALIGN_CENTER
        @desc Align text to the center.
        */
        ScriptUtils::declareConstant(vm, "_TEXT_ALIGN_CENTER", 2);
                /**SQConstant
        @name _TEXT_ALIGN_RIGHT
        @desc Align text to the right.
        */
        ScriptUtils::declareConstant(vm, "_TEXT_ALIGN_RIGHT", 3);
    }
}
