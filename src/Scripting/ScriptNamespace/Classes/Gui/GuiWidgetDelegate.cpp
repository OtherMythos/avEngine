#include "GuiWidgetDelegate.h"

#include "Scripting/ScriptNamespace/GuiNamespace.h"

#include "ColibriGui/ColibriWindow.h"
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
        ScriptUtils::addFunction(vm, setZOrder, "setZOrder", 2, ".i");
        ScriptUtils::addFunction(vm, setSkin, "setSkin", -2, ".si");
        ScriptUtils::addFunction(vm, setSkinPack, "setSkinPack", 2, ".s");

        ScriptUtils::addFunction(vm, createButton, "createButton");
        ScriptUtils::addFunction(vm, createLabel, "createLabel");
        ScriptUtils::addFunction(vm, createEditbox, "createEditbox");
        ScriptUtils::addFunction(vm, createSlider, "createSlider");
        ScriptUtils::addFunction(vm, createCheckbox, "createCheckbox");
        ScriptUtils::addFunction(vm, createWindow, "createWindow");

        ScriptUtils::addFunction(vm, getWidgetUserId, "getUserId");
        ScriptUtils::addFunction(vm, setWidgetUserId, "setUserId", 2, ".i");
    }

    void GuiWidgetDelegate::setupButton(HSQUIRRELVM vm){
        sq_newtableex(vm, 5);

        ScriptUtils::addFunction(vm, setPosition, "setPosition", 3, ".nn");
        ScriptUtils::addFunction(vm, setSize, "setSize", 3, ".nn");
        ScriptUtils::addFunction(vm, setHidden, "setHidden", 2, ".b");
        ScriptUtils::addFunction(vm, setZOrder, "setZOrder", 2, ".i");
        ScriptUtils::addFunction(vm, setSkin, "setSkin", -2, ".si");
        ScriptUtils::addFunction(vm, setSkinPack, "setSkinPack", 2, ".s");

        ScriptUtils::addFunction(vm, setText, "setText", -2, ".s|b");
        ScriptUtils::addFunction(vm, getText, "getText");
        ScriptUtils::addFunction(vm, sizeToFit, "sizeToFit");

        ScriptUtils::addFunction(vm, setDefaultFont, "setDefaultFont", 2, ".i");

        ScriptUtils::addFunction(vm, attachListener, "attachListener", -2, ".ct|x");
        ScriptUtils::addFunction(vm, detachListener, "detachListener");

        ScriptUtils::addFunction(vm, getWidgetUserId, "getUserId");
        ScriptUtils::addFunction(vm, setWidgetUserId, "setUserId", 2, ".i");
    }

    void GuiWidgetDelegate::setupLabel(HSQUIRRELVM vm){
        sq_newtableex(vm, 4);

        ScriptUtils::addFunction(vm, setPosition, "setPosition", 3, ".nn");
        ScriptUtils::addFunction(vm, setSize, "setSize", 3, ".nn");
        ScriptUtils::addFunction(vm, setHidden, "setHidden", 2, ".b");
        ScriptUtils::addFunction(vm, setZOrder, "setZOrder", 2, ".i");
        ScriptUtils::addFunction(vm, setSkin, "setSkin", -2, ".si");
        ScriptUtils::addFunction(vm, setSkinPack, "setSkinPack", 2, ".s");

        ScriptUtils::addFunction(vm, setText, "setText", -2, ".s|b");

        ScriptUtils::addFunction(vm, setDefaultFont, "setDefaultFont", 2, ".i");

        ScriptUtils::addFunction(vm, getWidgetUserId, "getUserId");
        ScriptUtils::addFunction(vm, setWidgetUserId, "setUserId", 2, ".i");
    }

    void GuiWidgetDelegate::setupEditbox(HSQUIRRELVM vm){
        sq_newtableex(vm, 4);

        ScriptUtils::addFunction(vm, setPosition, "setPosition", 3, ".nn");
        ScriptUtils::addFunction(vm, setSize, "setSize", 3, ".nn");
        ScriptUtils::addFunction(vm, setHidden, "setHidden", 2, ".b");
        ScriptUtils::addFunction(vm, setZOrder, "setZOrder", 2, ".i");
        ScriptUtils::addFunction(vm, setSkin, "setSkin", -2, ".si");
        ScriptUtils::addFunction(vm, setSkinPack, "setSkinPack", 2, ".s");

        ScriptUtils::addFunction(vm, setText, "setText", -2, ".s|b");
        ScriptUtils::addFunction(vm, getText, "getText");

        ScriptUtils::addFunction(vm, setDefaultFont, "setDefaultFont", 2, ".i");

        ScriptUtils::addFunction(vm, attachListener, "attachListener", -2, ".ct|x");
        ScriptUtils::addFunction(vm, detachListener, "detachListener");

        ScriptUtils::addFunction(vm, getWidgetUserId, "getUserId");
        ScriptUtils::addFunction(vm, setWidgetUserId, "setUserId", 2, ".i");
    }

    void GuiWidgetDelegate::setupSlider(HSQUIRRELVM vm){
        sq_newtableex(vm, 7);

        ScriptUtils::addFunction(vm, setPosition, "setPosition", 3, ".nn");
        ScriptUtils::addFunction(vm, setSize, "setSize", 3, ".nn");
        ScriptUtils::addFunction(vm, setHidden, "setHidden", 2, ".b");
        ScriptUtils::addFunction(vm, setZOrder, "setZOrder", 2, ".i");
        ScriptUtils::addFunction(vm, setSkin, "setSkin", -2, ".si");
        ScriptUtils::addFunction(vm, setSkinPack, "setSkinPack", 2, ".s");

        ScriptUtils::addFunction(vm, setSliderValue, "setValue", 2, ".f");
        ScriptUtils::addFunction(vm, getSliderValue, "getValue");

        ScriptUtils::addFunction(vm, attachListener, "attachListener", -2, ".ct|x");
        ScriptUtils::addFunction(vm, detachListener, "detachListener");

        ScriptUtils::addFunction(vm, getWidgetUserId, "getUserId");
        ScriptUtils::addFunction(vm, setWidgetUserId, "setUserId", 2, ".i");
    }

    void GuiWidgetDelegate::setupCheckbox(HSQUIRRELVM vm){
        sq_newtableex(vm, 7);

        ScriptUtils::addFunction(vm, setPosition, "setPosition", 3, ".nn");
        ScriptUtils::addFunction(vm, setSize, "setSize", 3, ".nn");
        ScriptUtils::addFunction(vm, setHidden, "setHidden", 2, ".b");
        ScriptUtils::addFunction(vm, setZOrder, "setZOrder", 2, ".i");
        ScriptUtils::addFunction(vm, setSkin, "setSkin", -2, ".si");
        ScriptUtils::addFunction(vm, setSkinPack, "setSkinPack", 2, ".s");

        ScriptUtils::addFunction(vm, setText, "setText", -2, ".s|b");

        ScriptUtils::addFunction(vm, setDefaultFont, "setDefaultFont", 2, ".i");

        ScriptUtils::addFunction(vm, setCheckboxValue, "setValue", 2, ".b");
        ScriptUtils::addFunction(vm, getCheckboxValue, "getValue");

        ScriptUtils::addFunction(vm, attachListener, "attachListener", -2, ".ct|x");
        ScriptUtils::addFunction(vm, detachListener, "detachListener");

        ScriptUtils::addFunction(vm, getWidgetUserId, "getUserId");
        ScriptUtils::addFunction(vm, setWidgetUserId, "setUserId", 2, ".i");
    }

    SQInteger GuiWidgetDelegate::setPosition(HSQUIRRELVM vm){
        SQFloat x, y;
        sq_getfloat(vm, -1, &y);
        sq_getfloat(vm, -2, &x);

        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, -3, &widget, &foundType));
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
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, -3, &widget, &foundType));
        if(!GuiNamespace::isTypeTagWidget(foundType)) return 0;

        widget->setSize(Ogre::Vector2(x, y));

        return 0;
    }

    SQInteger GuiWidgetDelegate::setHidden(HSQUIRRELVM vm){
        SQBool value;
        sq_getbool(vm, -1, &value);

        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, -2, &widget, &foundType));
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
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
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
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        if(!GuiNamespace::isTypeTagBasicWidget(foundType)) return 0;

        const char* str = 0;
        if(foundType == WidgetEditboxTypeTag){
            Colibri::Editbox* editbox = ((Colibri::Editbox*)widget);
            str = editbox->getLabel()->getText().c_str();
        }
        else if(foundType == WidgetButtonTypeTag){
            Colibri::Button* editbox = ((Colibri::Button*)widget);
            str = editbox->getLabel()->getText().c_str();
        }

        assert(str);
        sq_pushstring(vm, str, -1);

        return 1;
    }

    SQInteger GuiWidgetDelegate::setSliderValue(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        if(foundType != WidgetSliderTypeTag) return 0;

        SQFloat value;
        sq_getfloat(vm, 2, &value);

        ((Colibri::Slider*)widget)->setCurrentValue(value);

        return 0;
    }

    SQInteger GuiWidgetDelegate::getSliderValue(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        if(foundType != WidgetSliderTypeTag) return 0;

        float retVal = ((Colibri::Slider*)widget)->getCurrentValueRaw();

        sq_pushfloat(vm, retVal);

        return 1;
    }

    SQInteger GuiWidgetDelegate::getCheckboxValue(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        assert(foundType == WidgetCheckboxTypeTag);

        uint8_t retVal = ((Colibri::Checkbox*)widget)->getCurrentValue();

        sq_pushbool(vm, retVal > 0 ? true : false);

        return 1;
    }

    SQInteger GuiWidgetDelegate::setCheckboxValue(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        assert(foundType == WidgetCheckboxTypeTag);

        SQBool value;
        sq_getbool(vm, 2, &value);

        ((Colibri::Checkbox*)widget)->setCurrentValue(value ? 1 : 0);

        return 0;
    }

    SQInteger GuiWidgetDelegate::sizeToFit(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        if(!GuiNamespace::isTypeTagBasicWidget(foundType)) return 0;

        ((Colibri::Button*)widget)->sizeToFit();

        return 0;
    }

    SQInteger GuiWidgetDelegate::createLabel(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType));
        if(foundType != WidgetWindowTypeTag) return 0;

        assert(parent->isWindow());
        GuiNamespace::createWidget(vm, parent, GuiNamespace::WidgetType::Label);

        return 1;
    }

    SQInteger GuiWidgetDelegate::createButton(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType));
        if(foundType != WidgetWindowTypeTag) return 0;

        assert(parent->isWindow());
        GuiNamespace::createWidget(vm, parent, GuiNamespace::WidgetType::Button);

        return 1;
    }

    SQInteger GuiWidgetDelegate::createEditbox(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType));
        if(foundType != WidgetWindowTypeTag) return 0;

        assert(parent->isWindow());
        GuiNamespace::createWidget(vm, parent, GuiNamespace::WidgetType::Editbox);

        return 1;
    }

    SQInteger GuiWidgetDelegate::createSlider(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType));
        if(foundType != WidgetWindowTypeTag) return 0;

        assert(parent->isWindow());
        GuiNamespace::createWidget(vm, parent, GuiNamespace::WidgetType::Slider);

        return 1;
    }

    SQInteger GuiWidgetDelegate::createCheckbox(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType));
        if(foundType != WidgetWindowTypeTag) return 0;

        assert(parent->isWindow());
        GuiNamespace::createWidget(vm, parent, GuiNamespace::WidgetType::Checkbox);

        return 1;
    }

    SQInteger GuiWidgetDelegate::createWindow(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType));
        if(foundType != WidgetWindowTypeTag) return 0;

        assert(parent->isWindow());
        Colibri::Window* window = dynamic_cast<Colibri::Window*>(parent);
        assert(window);

        GuiNamespace::createWindow(vm, window);

        return 1;
    }

    SQInteger GuiWidgetDelegate::attachListener(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        assert(GuiNamespace::isTypeTagBasicWidget(foundType));

        if(sq_gettype(vm, 2) != OT_CLOSURE) return sq_throwerror(vm, "AttachListener expects a closure objec.");

        SQInteger top = sq_gettop(vm);

        SQObject targetFunction;
        sq_resetobject(&targetFunction);
        sq_getstackobj(vm, 2, &targetFunction);

        SQObject targetContext;
        sq_resetobject(&targetContext);
        if(top == 3){
            sq_getstackobj(vm, 3, &targetContext);
        }

        SQInteger numParams, numFreeVariables;
        sq_getclosureinfo(vm, 2, &numParams, &numFreeVariables);
        if(numParams != 3) return sq_throwerror(vm, "Listener function must have arguments (widget, action)");

        GuiNamespace::WidgetType type = GuiNamespace::getWidgetTypeFromTypeTag(foundType);
        assert(type != GuiNamespace::WidgetType::Unknown);
        GuiNamespace::registerWidgetListener(widget, targetFunction, targetContext, type);

        return 0;
    }

    SQInteger GuiWidgetDelegate::detachListener(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        if(!GuiNamespace::isTypeTagBasicWidget(foundType)) return 0; //Has to be a widget, but can't be a window.

        GuiNamespace::unbindWidgetListener(widget);

        return 0;
    }

    SQInteger GuiWidgetDelegate::getWidgetUserId(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        GuiNamespace::GuiWidgetUserData* data;
        bool result = GuiNamespace::getWidgetData(widget, &data);
        if(!result) return sq_throwerror(vm, "Invalid widget");

        sq_pushinteger(vm, data->userIdx);
        return 1;
    }

    SQInteger GuiWidgetDelegate::setWidgetUserId(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        GuiNamespace::GuiWidgetUserData* data;
        bool result = GuiNamespace::getWidgetData(widget, &data);
        if(!result) return sq_throwerror(vm, "Invalid widget");

        SQInteger id;
        sq_getinteger(vm, 2, &id);
        data->userIdx = id;

        return 0;
    }

    SQInteger GuiWidgetDelegate::setZOrder(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));

        SQInteger id;
        sq_getinteger(vm, 2, &id);
        //The last 55 are reserved for debug window stuff.
        if(id < 0 || id > 200) return sq_throwerror(vm, "Widget zOrder must be in range 0 - 200");

        widget->setZOrder(static_cast<uint8_t>(id));

        return 0;
    }

    SQInteger GuiWidgetDelegate::setDefaultFont(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));

        SQInteger id;
        sq_getinteger(vm, 2, &id);

        Colibri::Label* targetLabel = 0;
        if(foundType == WidgetButtonTypeTag){
            Colibri::Button* b = dynamic_cast<Colibri::Button*>(widget);
            assert(b);
            targetLabel = b->getLabel();
        }else if(foundType == WidgetLabelTypeTag){
            Colibri::Label* l = dynamic_cast<Colibri::Label*>(widget);
            assert(l);
            targetLabel = l;
        }else if(foundType == WidgetCheckboxTypeTag){
            Colibri::Checkbox* c = dynamic_cast<Colibri::Checkbox*>(widget);
            assert(c);
            targetLabel = c->getButton()->getLabel();
        }else if(foundType == WidgetEditboxTypeTag){
            Colibri::Editbox* e = dynamic_cast<Colibri::Editbox*>(widget);
            assert(e);
            targetLabel = e->getLabel();
        }else{
            return sq_throwerror(vm, "Invalid widget");
        }

        //TODO would be nice to have a warning if the user has provided a bad font.
        targetLabel->setDefaultFont(static_cast<uint16_t>(id));

        return 0;
    }

    SQInteger GuiWidgetDelegate::setSkin(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));

        const SQChar *text;
        sq_getstring(vm, 2, &text);

        //To set to all states if not.
        Colibri::States::States targetState = Colibri::States::NumStates;
        SQInteger stackSize = sq_gettop(vm);
        if(stackSize >= 3){
            SQInteger idx;
            sq_getinteger(vm, 3, &idx);
            if(idx < 0 || idx >= Colibri::States::NumStates) return sq_throwerror(vm, "Invalid state value.");
            targetState = static_cast<Colibri::States::States>(idx);
        }

        Colibri::Renderable* renderable = dynamic_cast<Colibri::Renderable*>(widget);
        assert(renderable);
        renderable->setSkin(text, targetState);

        return 0;
    }

    SQInteger GuiWidgetDelegate::setSkinPack(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));

        const SQChar *text;
        sq_getstring(vm, 2, &text);

        Colibri::Renderable* renderable = dynamic_cast<Colibri::Renderable*>(widget);
        assert(renderable);
        renderable->setSkinPack(text);

        return 0;
    }
}
