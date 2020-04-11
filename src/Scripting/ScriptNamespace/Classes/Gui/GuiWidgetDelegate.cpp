#include "GuiWidgetDelegate.h"

#include "Scripting/ScriptNamespace/GuiNamespace.h"

#include "ColibriGui/ColibriWidget.h"

namespace AV{

    void GuiWidgetDelegate::setupTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 1);

        ScriptUtils::addFunction(vm, setPosition, "setPosition", 3, ".nn");
        ScriptUtils::addFunction(vm, setSize, "setSize", 3, ".nn");
        ScriptUtils::addFunction(vm, setHidden, "setHidden", 2, ".b");
    }

    SQInteger GuiWidgetDelegate::setPosition(HSQUIRRELVM vm){
        SQFloat x, y;
        sq_getfloat(vm, -1, &y);
        sq_getfloat(vm, -2, &x);

        Colibri::Widget* widget = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, -3, &widget);
        if(result != USER_DATA_GET_SUCCESS) return 0;

        widget->setTopLeft(Ogre::Vector2(x, y));

        return 0;
    }

    SQInteger GuiWidgetDelegate::setSize(HSQUIRRELVM vm){
        SQFloat x, y;
        sq_getfloat(vm, -1, &y);
        sq_getfloat(vm, -2, &x);

        Colibri::Widget* widget = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, -3, &widget);
        if(result != USER_DATA_GET_SUCCESS) return 0;

        widget->setSize(Ogre::Vector2(x, y));

        return 0;
    }

    SQInteger GuiWidgetDelegate::setHidden(HSQUIRRELVM vm){
        SQBool value;
        sq_getbool(vm, -1, &value);

        Colibri::Widget* widget = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, -2, &widget);
        if(result != USER_DATA_GET_SUCCESS) return 0;

        widget->setHidden(value);

        return 0;
    }
}
