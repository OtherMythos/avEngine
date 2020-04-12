#include "GuiSizerDelegate.h"

#include "Scripting/ScriptNamespace/GuiNamespace.h"

#include "ColibriGui/ColibriWidget.h"
#include "ColibriGui/ColibriButton.h"
#include "ColibriGui/ColibriLabel.h"
#include "ColibriGui/Layouts/ColibriLayoutLine.h"

namespace AV{

    void GuiSizerDelegate::setupLayoutLine(HSQUIRRELVM vm){
        sq_newtableex(vm, 1);

        ScriptUtils::addFunction(vm, layout, "layout");
        ScriptUtils::addFunction(vm, addCell, "addCell", 2, ".u");
    }

    SQInteger GuiSizerDelegate::addCell(HSQUIRRELVM vm){
        Colibri::LayoutBase* layout = 0;
        UserDataGetResult result = GuiNamespace::getLayoutFromUserData(vm, -2, &layout);
        if(result != USER_DATA_GET_SUCCESS) return 0;

        Colibri::Widget* widget = 0;
        void* foundType = 0;
        result = GuiNamespace::getWidgetFromUserData(vm, -1, &widget, &foundType);
        if(result != USER_DATA_GET_SUCCESS) return 0;
        if(!GuiNamespace::isTypeTagBasicWidget(foundType)) return 0;

        ((Colibri::LayoutLine*)layout)->addCell(widget);

        return 0;
    }

    SQInteger GuiSizerDelegate::layout(HSQUIRRELVM vm){
        Colibri::LayoutBase* layout = 0;
        UserDataGetResult result = GuiNamespace::getLayoutFromUserData(vm, -1, &layout);
        if(result != USER_DATA_GET_SUCCESS) return 0;

        ((Colibri::LayoutLine*)layout)->layout();

        return 0;
    }
}
