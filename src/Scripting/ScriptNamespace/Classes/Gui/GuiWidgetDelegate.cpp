#include "GuiWidgetDelegate.h"

#include "Scripting/ScriptNamespace/GuiNamespace.h"

#include "ColibriGui/ColibriWidget.h"
#include "ColibriGui/ColibriButton.h"
#include "ColibriGui/ColibriLabel.h"

namespace AV{

    void GuiWidgetDelegate::setupWindow(HSQUIRRELVM vm){
        sq_newtableex(vm, 4);

        ScriptUtils::addFunction(vm, setPosition, "setPosition", 3, ".nn");
        ScriptUtils::addFunction(vm, setSize, "setSize", 3, ".nn");
        ScriptUtils::addFunction(vm, setHidden, "setHidden", 2, ".b");

        ScriptUtils::addFunction(vm, createButton, "createButton");
    }

    void GuiWidgetDelegate::setupButton(HSQUIRRELVM vm){
        sq_newtableex(vm, 3);

        ScriptUtils::addFunction(vm, setPosition, "setPosition", 3, ".nn");
        ScriptUtils::addFunction(vm, setSize, "setSize", 3, ".nn");
        ScriptUtils::addFunction(vm, setHidden, "setHidden", 2, ".b");

        ScriptUtils::addFunction(vm, setText, "setText", 2, ".s");
        ScriptUtils::addFunction(vm, sizeToFit, "sizeToFit");
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

    SQInteger GuiWidgetDelegate::setText(HSQUIRRELVM vm){
        const SQChar *text;
        sq_getstring(vm, -1, &text);

        Colibri::Widget* widget = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, -2, &widget);
        if(result != USER_DATA_GET_SUCCESS) return 0;

        ((Colibri::Button*)widget)->getLabel()->setText(text);

        return 0;
    }

    SQInteger GuiWidgetDelegate::sizeToFit(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, 1, &widget);
        if(result != USER_DATA_GET_SUCCESS) return 0;

        ((Colibri::Button*)widget)->sizeToFit();

        return 0;
    }


    SQInteger GuiWidgetDelegate::createButton(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, 1, &parent);
        if(result != USER_DATA_GET_SUCCESS) return 0;

        assert(parent->isWindow());
        GuiNamespace::createWidget(vm, parent, GuiNamespace::WidgetType::Button);

        return 1;
    }
}
