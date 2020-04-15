#include "GuiWidgetDelegate.h"

#include "Scripting/ScriptNamespace/GuiNamespace.h"

#include "ColibriGui/ColibriWidget.h"
#include "ColibriGui/ColibriButton.h"
#include "ColibriGui/ColibriLabel.h"

#include "Scripting/ScriptObjectTypeTags.h"

namespace AV{

    void GuiWidgetDelegate::setupWindow(HSQUIRRELVM vm){
        sq_newtableex(vm, 5);

        ScriptUtils::addFunction(vm, setPosition, "setPosition", 3, ".nn");
        ScriptUtils::addFunction(vm, setSize, "setSize", 3, ".nn");
        ScriptUtils::addFunction(vm, setHidden, "setHidden", 2, ".b");

        ScriptUtils::addFunction(vm, createButton, "createButton");
        ScriptUtils::addFunction(vm, createLabel, "createLabel");
    }

    void GuiWidgetDelegate::setupButton(HSQUIRRELVM vm){
        sq_newtableex(vm, 5);

        ScriptUtils::addFunction(vm, setPosition, "setPosition", 3, ".nn");
        ScriptUtils::addFunction(vm, setSize, "setSize", 3, ".nn");
        ScriptUtils::addFunction(vm, setHidden, "setHidden", 2, ".b");

        ScriptUtils::addFunction(vm, setText, "setText", -2, ".s|b");
        ScriptUtils::addFunction(vm, sizeToFit, "sizeToFit");
        ScriptUtils::addFunction(vm, attachListener, "attachListener", 2, ".c");
    }

    void GuiWidgetDelegate::setupLabel(HSQUIRRELVM vm){
        sq_newtableex(vm, 4);

        ScriptUtils::addFunction(vm, setPosition, "setPosition", 3, ".nn");
        ScriptUtils::addFunction(vm, setSize, "setSize", 3, ".nn");
        ScriptUtils::addFunction(vm, setHidden, "setHidden", 2, ".b");

        ScriptUtils::addFunction(vm, setText, "setText", -2, ".s|b");
    }

    SQInteger GuiWidgetDelegate::setPosition(HSQUIRRELVM vm){
        SQFloat x, y;
        sq_getfloat(vm, -1, &y);
        sq_getfloat(vm, -2, &x);

        Colibri::Widget* widget = 0;
        void* foundType = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, -3, &widget, &foundType);
        if(result != USER_DATA_GET_SUCCESS) return 0;
        if(!GuiNamespace::isTypeTagWidget(foundType)) return 0;

        widget->setTopLeft(Ogre::Vector2(x, y));

        return 0;
    }

    SQInteger GuiWidgetDelegate::setSize(HSQUIRRELVM vm){
        SQFloat x, y;
        sq_getfloat(vm, -1, &y);
        sq_getfloat(vm, -2, &x);

        Colibri::Widget* widget = 0;
        void* foundType = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, -3, &widget, &foundType);
        if(result != USER_DATA_GET_SUCCESS) return 0;
        if(!GuiNamespace::isTypeTagWidget(foundType)) return 0;

        widget->setSize(Ogre::Vector2(x, y));

        return 0;
    }

    SQInteger GuiWidgetDelegate::setHidden(HSQUIRRELVM vm){
        SQBool value;
        sq_getbool(vm, -1, &value);

        Colibri::Widget* widget = 0;
        void* foundType = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, -2, &widget, &foundType);
        if(result != USER_DATA_GET_SUCCESS) return 0;
        if(!GuiNamespace::isTypeTagWidget(foundType)) return 0;

        widget->setHidden(value);

        return 0;
    }

    SQInteger GuiWidgetDelegate::setText(HSQUIRRELVM vm){
        const SQChar *text;
        sq_getstring(vm, 2, &text);

        SQBool shouldSizeToFit = true;
        SQInteger stackSize = sq_gettop(vm);
        if(stackSize >= 3){
            sq_getbool(vm, 3, &shouldSizeToFit);
        }

        Colibri::Widget* widget = 0;
        void* foundType = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType);
        if(result != USER_DATA_GET_SUCCESS) return 0;
        if(!GuiNamespace::isTypeTagBasicWidget(foundType)) return 0;

        if(foundType == WidgetLabelTypeTag){
            Colibri::Label* label = ((Colibri::Label*)widget);
            label->setText(text);
            if(shouldSizeToFit) label->sizeToFit();
        }else{
            assert(foundType == WidgetButtonTypeTag); //At this point there's only two possible widgets.
            Colibri::Button* button = ((Colibri::Button*)widget);
            button->getLabel()->setText(text);
            if(shouldSizeToFit) button->sizeToFit();
        }

        return 0;
    }

    SQInteger GuiWidgetDelegate::sizeToFit(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType);
        if(result != USER_DATA_GET_SUCCESS) return 0;
        if(!GuiNamespace::isTypeTagBasicWidget(foundType)) return 0;

        ((Colibri::Button*)widget)->sizeToFit();

        return 0;
    }

    SQInteger GuiWidgetDelegate::createLabel(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType);
        if(result != USER_DATA_GET_SUCCESS) return 0;
        if(foundType != WidgetWindowTypeTag) return 0;

        assert(parent->isWindow());
        GuiNamespace::createWidget(vm, parent, GuiNamespace::WidgetType::Label);

        return 1;
    }

    SQInteger GuiWidgetDelegate::createButton(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType);
        if(result != USER_DATA_GET_SUCCESS) return 0;
        if(foundType != WidgetWindowTypeTag) return 0;

        assert(parent->isWindow());
        GuiNamespace::createWidget(vm, parent, GuiNamespace::WidgetType::Button);

        return 1;
    }

    SQInteger GuiWidgetDelegate::attachListener(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, -2, &widget, &foundType);
        if(result != USER_DATA_GET_SUCCESS) return 0;
        if(!GuiNamespace::isTypeTagBasicWidget(foundType)) return 0; //Has to be a widget, but can't be a window.

        if(sq_gettype(vm, -1) != OT_CLOSURE) return 0; //Can't be a native closure or anything else.

        SQObject targetFunction;
        sq_resetobject(&targetFunction);
        sq_getstackobj(vm, -1, &targetFunction);

        GuiNamespace::registerWidgetListener(vm, widget, targetFunction);

        return 0;
    }
}
