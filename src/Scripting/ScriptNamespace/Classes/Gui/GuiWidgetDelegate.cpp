#include "GuiWidgetDelegate.h"

#include "Scripting/ScriptNamespace/GuiNamespace.h"

#include "ColibriGui/ColibriWidget.h"
#include "ColibriGui/ColibriButton.h"
#include "ColibriGui/ColibriLabel.h"
#include "ColibriGui/ColibriEditbox.h"
#include "ColibriGui/ColibriSlider.h"
#include "ColibriGui/ColibriCheckbox.h"

#include "Scripting/ScriptObjectTypeTags.h"

namespace AV{

    void GuiWidgetDelegate::setupWindow(HSQUIRRELVM vm){
        sq_newtableex(vm, 5);

        ScriptUtils::addFunction(vm, setPosition, "setPosition", 3, ".nn");
        ScriptUtils::addFunction(vm, setSize, "setSize", 3, ".nn");
        ScriptUtils::addFunction(vm, setHidden, "setHidden", 2, ".b");

        ScriptUtils::addFunction(vm, createButton, "createButton");
        ScriptUtils::addFunction(vm, createLabel, "createLabel");
        ScriptUtils::addFunction(vm, createEditbox, "createEditbox");
        ScriptUtils::addFunction(vm, createSlider, "createSlider");
        ScriptUtils::addFunction(vm, createCheckbox, "createCheckbox");
    }

    void GuiWidgetDelegate::setupButton(HSQUIRRELVM vm){
        sq_newtableex(vm, 5);

        ScriptUtils::addFunction(vm, setPosition, "setPosition", 3, ".nn");
        ScriptUtils::addFunction(vm, setSize, "setSize", 3, ".nn");
        ScriptUtils::addFunction(vm, setHidden, "setHidden", 2, ".b");

        ScriptUtils::addFunction(vm, setText, "setText", -2, ".s|b");
        ScriptUtils::addFunction(vm, sizeToFit, "sizeToFit");

        ScriptUtils::addFunction(vm, attachListener, "attachListener", 2, ".c");
        ScriptUtils::addFunction(vm, detachListener, "detachListener");
    }

    void GuiWidgetDelegate::setupLabel(HSQUIRRELVM vm){
        sq_newtableex(vm, 4);

        ScriptUtils::addFunction(vm, setPosition, "setPosition", 3, ".nn");
        ScriptUtils::addFunction(vm, setSize, "setSize", 3, ".nn");
        ScriptUtils::addFunction(vm, setHidden, "setHidden", 2, ".b");

        ScriptUtils::addFunction(vm, setText, "setText", -2, ".s|b");
    }

    void GuiWidgetDelegate::setupEditbox(HSQUIRRELVM vm){
        sq_newtableex(vm, 4);

        ScriptUtils::addFunction(vm, setPosition, "setPosition", 3, ".nn");
        ScriptUtils::addFunction(vm, setSize, "setSize", 3, ".nn");
        ScriptUtils::addFunction(vm, setHidden, "setHidden", 2, ".b");

        ScriptUtils::addFunction(vm, setText, "setText", -2, ".s|b");
        ScriptUtils::addFunction(vm, getText, "getText");

        ScriptUtils::addFunction(vm, attachListener, "attachListener", 2, ".c");
        ScriptUtils::addFunction(vm, detachListener, "detachListener");
    }

    void GuiWidgetDelegate::setupSlider(HSQUIRRELVM vm){
        sq_newtableex(vm, 7);

        ScriptUtils::addFunction(vm, setPosition, "setPosition", 3, ".nn");
        ScriptUtils::addFunction(vm, setSize, "setSize", 3, ".nn");
        ScriptUtils::addFunction(vm, setHidden, "setHidden", 2, ".b");

        ScriptUtils::addFunction(vm, setSliderValue, "setValue", 2, ".f");
        ScriptUtils::addFunction(vm, getSliderValue, "getValue");

        ScriptUtils::addFunction(vm, attachListener, "attachListener", 2, ".c");
        ScriptUtils::addFunction(vm, detachListener, "detachListener");
    }

    void GuiWidgetDelegate::setupCheckbox(HSQUIRRELVM vm){
        sq_newtableex(vm, 7);

        ScriptUtils::addFunction(vm, setPosition, "setPosition", 3, ".nn");
        ScriptUtils::addFunction(vm, setSize, "setSize", 3, ".nn");
        ScriptUtils::addFunction(vm, setHidden, "setHidden", 2, ".b");

        ScriptUtils::addFunction(vm, setText, "setText", -2, ".s|b");

        ScriptUtils::addFunction(vm, setCheckboxValue, "setValue", 2, ".b");
        ScriptUtils::addFunction(vm, getCheckboxValue, "getValue");

        ScriptUtils::addFunction(vm, attachListener, "attachListener", 2, ".c");
        ScriptUtils::addFunction(vm, detachListener, "detachListener");
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
        }else if(foundType == WidgetEditboxTypeTag){
            Colibri::Editbox* editbox = ((Colibri::Editbox*)widget);
            editbox->setText(text);
        }else if(foundType == WidgetCheckboxTypeTag){
            Colibri::Checkbox* checkbox = ((Colibri::Checkbox*)widget);
            checkbox->getButton()->getLabel()->setText(text);
            if(shouldSizeToFit) checkbox->sizeToFit();
        }else{
            Colibri::Button* button = ((Colibri::Button*)widget);
            button->getLabel()->setText(text);
            if(shouldSizeToFit) button->sizeToFit();
        }

        return 0;
    }

    SQInteger GuiWidgetDelegate::getText(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType);
        if(result != USER_DATA_GET_SUCCESS) return 0;
        if(!GuiNamespace::isTypeTagBasicWidget(foundType)) return 0;

        const char* str = 0;
        if(foundType == WidgetEditboxTypeTag){
            Colibri::Editbox* editbox = ((Colibri::Editbox*)widget);
            str = editbox->getLabel()->getText().c_str();
        }

        assert(str);
        sq_pushstring(vm, str, -1);

        return 1;
    }

    SQInteger GuiWidgetDelegate::setSliderValue(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType);
        if(result != USER_DATA_GET_SUCCESS) return 0;
        if(foundType != WidgetSliderTypeTag) return 0;

        SQFloat value;
        sq_getfloat(vm, 2, &value);

        ((Colibri::Slider*)widget)->setValue(value);

        return 0;
    }

    SQInteger GuiWidgetDelegate::getSliderValue(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType);
        if(result != USER_DATA_GET_SUCCESS) return 0;
        if(foundType != WidgetSliderTypeTag) return 0;

        float retVal = ((Colibri::Slider*)widget)->getValue();

        sq_pushfloat(vm, retVal);

        return 1;
    }

    SQInteger GuiWidgetDelegate::getCheckboxValue(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType);
        if(result != USER_DATA_GET_SUCCESS) return 0;
        assert(foundType == WidgetCheckboxTypeTag);

        uint8_t retVal = ((Colibri::Checkbox*)widget)->getCurrentValue();

        sq_pushbool(vm, retVal > 0 ? true : false);

        return 1;
    }

    SQInteger GuiWidgetDelegate::setCheckboxValue(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType);
        if(result != USER_DATA_GET_SUCCESS) return 0;
        assert(foundType == WidgetCheckboxTypeTag);

        SQBool value;
        sq_getbool(vm, 2, &value);

        ((Colibri::Checkbox*)widget)->setCurrentValue(value ? 1 : 0);

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

    SQInteger GuiWidgetDelegate::createEditbox(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType);
        if(result != USER_DATA_GET_SUCCESS) return 0;
        if(foundType != WidgetWindowTypeTag) return 0;

        assert(parent->isWindow());
        GuiNamespace::createWidget(vm, parent, GuiNamespace::WidgetType::Editbox);

        return 1;
    }

    SQInteger GuiWidgetDelegate::createSlider(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType);
        if(result != USER_DATA_GET_SUCCESS) return 0;
        if(foundType != WidgetWindowTypeTag) return 0;

        assert(parent->isWindow());
        GuiNamespace::createWidget(vm, parent, GuiNamespace::WidgetType::Slider);

        return 1;
    }

    SQInteger GuiWidgetDelegate::createCheckbox(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType);
        if(result != USER_DATA_GET_SUCCESS) return 0;
        if(foundType != WidgetWindowTypeTag) return 0;

        assert(parent->isWindow());
        GuiNamespace::createWidget(vm, parent, GuiNamespace::WidgetType::Checkbox);

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

        GuiNamespace::WidgetType type = GuiNamespace::getWidgetTypeFromTypeTag(foundType);
        assert(type != GuiNamespace::WidgetType::Unknown);
        GuiNamespace::registerWidgetListener(widget, targetFunction, type);

        return 0;
    }

    SQInteger GuiWidgetDelegate::detachListener(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        UserDataGetResult result = GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType);
        if(result != USER_DATA_GET_SUCCESS) return 0;
        if(!GuiNamespace::isTypeTagBasicWidget(foundType)) return 0; //Has to be a widget, but can't be a window.

        GuiNamespace::unbindWidgetListener(widget);

        return 0;
    }
}