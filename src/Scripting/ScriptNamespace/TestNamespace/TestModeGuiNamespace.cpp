#ifdef TEST_MODE

#include "TestModeGuiNamespace.h"

#include "Scripting/ScriptNamespace/GuiNamespace.h"

namespace AV{
    SQInteger TestModeGuiNamespace::getNumWindows(HSQUIRRELVM vm){
        int num = GuiNamespace::getNumWindows();

        sq_pushinteger(vm, num);

        return 1;
    }

    SQInteger TestModeGuiNamespace::getNumWidgets(HSQUIRRELVM vm){
        int num = GuiNamespace::getNumWidgets();

        sq_pushinteger(vm, num);

        return 1;
    }

    SQInteger TestModeGuiNamespace::callListener(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, -2, &widget, &foundType));
        if(!GuiNamespace::isTypeTagBasicWidget(foundType)) return 0;

        SQInteger action;
        sq_getinteger(vm, -1, &action);

        GuiNamespace::_notifyWidgetActionPerformed(widget, (Colibri::Action::Action)action);

        return 0;
    }

    void TestModeGuiNamespace::setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled){
        ScriptUtils::RedirectFunctionMap functionMap;
        functionMap["getNumWindows"] = {"", 0, getNumWindows};
        functionMap["getNumWidgets"] = {"", 0, getNumWidgets};
        functionMap["callListener"] = {".ui", 0, callListener};

        ScriptUtils::redirectFunctionMap(vm, messageFunction, functionMap, testModeEnabled);
    }
}

#endif
