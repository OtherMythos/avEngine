#include "GuiWidgetDelegate.h"

#include "Scripting/ScriptNamespace/GuiNamespace.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Hlms/DatablockUserData.h"

#include "ColibriGui/ColibriWindow.h"
#include "ColibriGui/ColibriWidget.h"
#include "ColibriGui/ColibriButton.h"
#include "ColibriGui/ColibriLabel.h"
#include "ColibriGui/ColibriSpinner.h"
#include "ColibriGui/ColibriEditbox.h"
#include "ColibriGui/ColibriSlider.h"
#include "ColibriGui/ColibriCheckbox.h"
#include "Gui/AnimatedLabel.h"

#include "Gui/WrappedColibriRenderable.h"
#include "OgreHlmsDatablock.h"

#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"
#include "Scripting/ScriptObjectTypeTags.h"
#include "Scripting/ScriptNamespace/Classes/Vector2UserData.h"

#include "OgreStringConverter.h"

namespace AV{

    #define BASIC_WIDGET_FUNCTIONS \
        ScriptUtils::addFunction(vm, setPosition, "setPosition", -2, ".u|nn"); \
        ScriptUtils::addFunction(vm, getPosition, "getPosition"); \
        ScriptUtils::addFunction(vm, getSize, "getSize"); \
        ScriptUtils::addFunction(vm, setSize, "setSize", -2, ".u|nn"); \
        ScriptUtils::addFunction(vm, setHidden, "setHidden", 2, ".b"); \
        ScriptUtils::addFunction(vm, setZOrder, "setZOrder", 2, ".i"); \
        ScriptUtils::addFunction(vm, setSkin, "setSkin", -2, ".si"); \
        ScriptUtils::addFunction(vm, setSkinPack, "setSkinPack", 2, ".s"); \
        ScriptUtils::addFunction(vm, setDatablock, "setDatablock", 2, ".u|s"); \
        ScriptUtils::addFunction(vm, setOrientation, "setOrientation", 2, ".f"); \
        ScriptUtils::addFunction(vm, getDatablock, "getDatablock"); \
        ScriptUtils::addFunction(vm, setClickable, "setClickable", 2, ".b"); \
        ScriptUtils::addFunction(vm, setKeyboardNavigable, "setKeyboardNavigable", 2, ".b"); \
        ScriptUtils::addFunction(vm, setKeyboardNavigable, "setKeyboardNavigable", 2, ".b"); \
        ScriptUtils::addFunction(vm, setFocus, "setFocus"); \
        ScriptUtils::addFunction(vm, setClipBorders, "setClipBorders", 5, ".nnnn"); \
        ScriptUtils::addFunction(vm, setVisualsEnabled, "setVisualsEnabled", 2, ".b"); \
        \
        ScriptUtils::addFunction(vm, getWidgetUserId, "getUserId"); \
        ScriptUtils::addFunction(vm, setWidgetUserId, "setUserId", 2, ".i");

    #define LISTENER_WIDGET_FUNCTIONS \
        ScriptUtils::addFunction(vm, attachListener, "attachListener", -2, ".ct|x"); \
        ScriptUtils::addFunction(vm, attachListenerForEvent, "attachListenerForEvent", -3, ".cit|x"); \
        ScriptUtils::addFunction(vm, detachListener, "detachListener");

    #define LABEL_WIDGET_FUNCTIONS \
        ScriptUtils::addFunction(vm, setDefaultFont, "setDefaultFont", 2, ".i"); \
        ScriptUtils::addFunction(vm, setDefaultFontSize, "setDefaultFontSize", 2, ".n"); \
        ScriptUtils::addFunction(vm, getDefaultFontSize, "getDefaultFontSize"); \
        ScriptUtils::addFunction(vm, setTextHorizontalAlignment, "setTextHorizontalAlignment", 2, ".i"); \
        ScriptUtils::addFunction(vm, setTextColour, "setTextColour", -4, ".nnnn"); \
        ScriptUtils::addFunction(vm, setRichText, "setRichText", -2, ".ai");

    #define CHECK_FOR_WINDOW \
        if(foundType != WidgetWindowTypeTag) return sq_throwerror(vm, "Expected widget of type window.");
    #define CHECK_FOR_SLIDER \
        if(foundType != WidgetSliderTypeTag) return sq_throwerror(vm, "Expected widget of type slider.");
    #define CHECK_FOR_CHECKBOX \
        if(foundType != WidgetCheckboxTypeTag) return sq_throwerror(vm, "Expected widget of type checkbox.");
    #define CHECK_FOR_SPINNER \
        if(foundType != WidgetSpinnerTypeTag) return sq_throwerror(vm, "Expected widget of type spinner.");
    #define CHECK_FOR_BASIC_WIDGET \
        if(!GuiNamespace::isTypeTagBasicWidget(foundType)) return sq_throwerror(vm, "Expected basic widget, not window.");

    void GuiWidgetDelegate::setupWindow(HSQUIRRELVM vm){
        sq_newtableex(vm, 5);

        BASIC_WIDGET_FUNCTIONS

        ScriptUtils::addFunction(vm, createButton, "createButton");
        ScriptUtils::addFunction(vm, createLabel, "createLabel");
        ScriptUtils::addFunction(vm, createAnimatedLabel, "createAnimatedLabel");
        ScriptUtils::addFunction(vm, createEditbox, "createEditbox");
        ScriptUtils::addFunction(vm, createSlider, "createSlider");
        ScriptUtils::addFunction(vm, createCheckbox, "createCheckbox");
        ScriptUtils::addFunction(vm, createWindow, "createWindow");
        ScriptUtils::addFunction(vm, createPanel, "createPanel");
        ScriptUtils::addFunction(vm, createSpinner, "createSpinner");

        ScriptUtils::addFunction(vm, windowSizeScrollToFit, "sizeScrollToFit");
        ScriptUtils::addFunction(vm, windowSetMaxScroll, "setMaxScroll", 3, ".nn");
        ScriptUtils::addFunction(vm, windowSetAllowMouseScroll, "setAllowMouseScroll", 2, ".b");
        ScriptUtils::addFunction(vm, windowSetConsumeCursor, "setConsumeCursor", 2, ".b");
    }

    void GuiWidgetDelegate::setupButton(HSQUIRRELVM vm){
        sq_newtableex(vm, 5);

        BASIC_WIDGET_FUNCTIONS
        LISTENER_WIDGET_FUNCTIONS
        LABEL_WIDGET_FUNCTIONS

        ScriptUtils::addFunction(vm, setText, "setText", -2, ".s|b");
        ScriptUtils::addFunction(vm, getText, "getText");
        ScriptUtils::addFunction(vm, sizeToFit, "sizeToFit");

    }

    void GuiWidgetDelegate::setupLabel(HSQUIRRELVM vm){
        sq_newtableex(vm, 4);

        BASIC_WIDGET_FUNCTIONS
        LABEL_WIDGET_FUNCTIONS

        ScriptUtils::addFunction(vm, setText, "setText", -2, ".s|b");

    }

    void GuiWidgetDelegate::setupAnimatedLabel(HSQUIRRELVM vm){
        sq_newtableex(vm, 4);

        BASIC_WIDGET_FUNCTIONS
        LABEL_WIDGET_FUNCTIONS

        ScriptUtils::addFunction(vm, setText, "setText", -2, ".s|b");
        ScriptUtils::addFunction(vm, setAnimatedGlyph, "setAnimatedGlyph", -2, ".inni");

    }

    void GuiWidgetDelegate::setupEditbox(HSQUIRRELVM vm){
        sq_newtableex(vm, 4);

        BASIC_WIDGET_FUNCTIONS
        LISTENER_WIDGET_FUNCTIONS
        LABEL_WIDGET_FUNCTIONS

        ScriptUtils::addFunction(vm, setText, "setText", -2, ".s|b");
        ScriptUtils::addFunction(vm, getText, "getText");

    }

    void GuiWidgetDelegate::setupSlider(HSQUIRRELVM vm){
        sq_newtableex(vm, 7);

        BASIC_WIDGET_FUNCTIONS
        LISTENER_WIDGET_FUNCTIONS

        ScriptUtils::addFunction(vm, setSliderValue, "setValue", 2, ".f");
        ScriptUtils::addFunction(vm, getSliderValue, "getValue");

        ScriptUtils::addFunction(vm, setSliderRange, "setRange", 3, ".ii");
    }

    void GuiWidgetDelegate::setupCheckbox(HSQUIRRELVM vm){
        sq_newtableex(vm, 7);

        BASIC_WIDGET_FUNCTIONS
        LISTENER_WIDGET_FUNCTIONS
        LABEL_WIDGET_FUNCTIONS

        ScriptUtils::addFunction(vm, setText, "setText", -2, ".s|b");

        ScriptUtils::addFunction(vm, setCheckboxValue, "setValue", 2, ".b");
        ScriptUtils::addFunction(vm, getCheckboxValue, "getValue");
    }

    void GuiWidgetDelegate::setupPanel(HSQUIRRELVM vm){
        sq_newtableex(vm, 7);

        BASIC_WIDGET_FUNCTIONS
        LISTENER_WIDGET_FUNCTIONS
    }


    void GuiWidgetDelegate::setupSpinner(HSQUIRRELVM vm){
        sq_newtableex(vm, 9);

        ScriptUtils::addFunction(vm, setSpinnerOptions, "setOptions", 2, ".a");
        ScriptUtils::addFunction(vm, setText, "setText", -2, ".s|b");

        ScriptUtils::addFunction(vm, getSpinnerValue, "getValue");
        ScriptUtils::addFunction(vm, getSpinnerValueRaw, "getValueRaw");
        ScriptUtils::addFunction(vm, setSpinnerValueRaw, "setValueRaw");

        BASIC_WIDGET_FUNCTIONS
        LISTENER_WIDGET_FUNCTIONS
    }

    #undef BASIC_WIDGET_FUNCTIONS
    #undef LISTENER_WIDGET_FUNCTIONS

    SQInteger GuiWidgetDelegate::getPosition(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        assert(GuiNamespace::isTypeTagWidget(foundType));

        Vector2UserData::vector2ToUserData(vm, widget->getLocalTopLeft());

        return 1;
    }

    SQInteger GuiWidgetDelegate::getSize(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        assert(GuiNamespace::isTypeTagWidget(foundType));

        Vector2UserData::vector2ToUserData(vm, widget->getSize());

        return 1;
    }

    SQInteger GuiWidgetDelegate::setPosition(HSQUIRRELVM vm){
        Ogre::Vector2 outVec;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read2FloatsOrVec2(vm, &outVec));

        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        assert(GuiNamespace::isTypeTagWidget(foundType));

        widget->setTopLeft(outVec);

        return 0;
    }

    SQInteger GuiWidgetDelegate::setSize(HSQUIRRELVM vm){
        Ogre::Vector2 outVec;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read2FloatsOrVec2(vm, &outVec));

        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        assert(GuiNamespace::isTypeTagWidget(foundType));

        widget->setSizeAndCellMinSize(outVec);

        return 0;
    }

    SQInteger GuiWidgetDelegate::setHidden(HSQUIRRELVM vm){
        SQBool value;
        sq_getbool(vm, -1, &value);

        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, -2, &widget, &foundType));
        assert(GuiNamespace::isTypeTagWidget(foundType));

        widget->setHidden(value);

        return 0;
    }

    SQInteger GuiWidgetDelegate::setAnimatedGlyph(HSQUIRRELVM vm){
        SQInteger glyphId;
        sq_getinteger(vm, 2, &glyphId);
        if(glyphId < 0) return sq_throwerror(vm, "Invalid glyphId");

        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        assert(foundType == WidgetAnimatedLabelTypeTag);

        SQFloat offsetX;
        sq_getfloat(vm, 3, &offsetX);
        SQFloat offsetY;
        sq_getfloat(vm, 4, &offsetY);

        SQInteger colourValue;
        sq_getinteger(vm, 5, &colourValue);

        uint32_t animColour = static_cast<uint32_t>(colourValue);

        AnimatedLabel* target = dynamic_cast<AnimatedLabel*>(widget);
        bool result = target->setGlyphAnimation(static_cast<uint32>(glyphId), animColour, offsetX, offsetY);
        if(!result) return sq_throwerror(vm, "Invalid glyph provided");

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
        CHECK_FOR_BASIC_WIDGET

        if(foundType == WidgetLabelTypeTag || foundType == WidgetAnimatedLabelTypeTag){
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
        }else if(foundType == WidgetButtonTypeTag){
            Colibri::Button* button = ((Colibri::Button*)widget);
            button->getLabel()->setText(text);
            if(shouldSizeToFit) button->sizeToFit();
        }else if(foundType == WidgetSpinnerTypeTag){
            Colibri::Spinner* spinner = ((Colibri::Spinner*)widget);
            spinner->getLabel()->setText(text);
            if(shouldSizeToFit) spinner->sizeToFit();
        }else{
            assert(false);
        }

        return 0;
    }

    SQInteger GuiWidgetDelegate::getText(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        CHECK_FOR_BASIC_WIDGET

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
        CHECK_FOR_SLIDER

        SQFloat value;
        sq_getfloat(vm, 2, &value);

        ((Colibri::Slider*)widget)->setCurrentValue(value);

        return 0;
    }

    SQInteger GuiWidgetDelegate::getSliderValue(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        CHECK_FOR_SLIDER

        float retVal = ((Colibri::Slider*)widget)->getCurrentValueRaw();

        sq_pushfloat(vm, retVal);

        return 1;
    }

    SQInteger GuiWidgetDelegate::getCheckboxValue(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        CHECK_FOR_CHECKBOX

        uint8_t retVal = ((Colibri::Checkbox*)widget)->getCurrentValue();

        sq_pushbool(vm, retVal > 0 ? true : false);

        return 1;
    }

    SQInteger GuiWidgetDelegate::setCheckboxValue(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        CHECK_FOR_CHECKBOX

        SQBool value;
        sq_getbool(vm, 2, &value);

        ((Colibri::Checkbox*)widget)->setCurrentValue(value ? 1 : 0);

        return 0;
    }


    SQInteger GuiWidgetDelegate::getSpinnerValue(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        CHECK_FOR_SPINNER

        std::string value = ((Colibri::Spinner*)widget)->getCurrentValueStr();
        sq_pushstring(vm, value.c_str(), -1);

        return 1;
    }

    SQInteger GuiWidgetDelegate::setSpinnerValueRaw(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        CHECK_FOR_SPINNER

        SQInteger value;
        sq_getinteger(vm, 2, &value);

        ((Colibri::Spinner*)widget)->setCurrentValue(value);

        return 0;
    }

    SQInteger GuiWidgetDelegate::getSpinnerValueRaw(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        CHECK_FOR_SPINNER

        int32_t value = ((Colibri::Spinner*)widget)->getCurrentValueRaw();
        sq_pushinteger(vm, value);

        return 1;
    }

    SQInteger GuiWidgetDelegate::setSpinnerOptions(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        CHECK_FOR_SPINNER

        std::vector<std::string> options;
        sq_pushnull(vm);
        while(SQ_SUCCEEDED(sq_next(vm, -2))){
            SQObjectType objectType = sq_gettype(vm, -1);
            if(objectType != OT_STRING){
                sq_pop(vm, 2);
                continue;
            }

            const SQChar *key;
            sq_getstring(vm, -2, &key);
            const SQChar *val;
            sq_getstring(vm, -1, &val);
            options.push_back(val);

            sq_pop(vm, 2);
        }
        sq_pop(vm, 1);

        ((Colibri::Spinner*)widget)->setOptions(options);
        ((Colibri::Spinner*)widget)->sizeToFit();

        return 0;
    }

    SQInteger GuiWidgetDelegate::sizeToFit(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        CHECK_FOR_BASIC_WIDGET

        ((Colibri::Button*)widget)->sizeToFit();

        return 0;
    }

    SQInteger GuiWidgetDelegate::createLabel(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType));
        CHECK_FOR_WINDOW

        assert(parent->isWindow());
        GuiNamespace::createWidget(vm, parent, GuiNamespace::WidgetType::Label);

        return 1;
    }

    SQInteger GuiWidgetDelegate::createAnimatedLabel(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType));
        CHECK_FOR_WINDOW

        assert(parent->isWindow());
        GuiNamespace::createWidget(vm, parent, GuiNamespace::WidgetType::AnimatedLabel);

        return 1;
    }

    SQInteger GuiWidgetDelegate::createButton(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType));
        CHECK_FOR_WINDOW

        assert(parent->isWindow());
        GuiNamespace::createWidget(vm, parent, GuiNamespace::WidgetType::Button);

        return 1;
    }

    SQInteger GuiWidgetDelegate::createEditbox(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType));
        CHECK_FOR_WINDOW

        assert(parent->isWindow());
        GuiNamespace::createWidget(vm, parent, GuiNamespace::WidgetType::Editbox);

        return 1;
    }

    SQInteger GuiWidgetDelegate::createSlider(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType));
        CHECK_FOR_WINDOW

        assert(parent->isWindow());
        GuiNamespace::createWidget(vm, parent, GuiNamespace::WidgetType::Slider);

        return 1;
    }

    SQInteger GuiWidgetDelegate::createCheckbox(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType));
        CHECK_FOR_WINDOW

        assert(parent->isWindow());
        GuiNamespace::createWidget(vm, parent, GuiNamespace::WidgetType::Checkbox);

        return 1;
    }

    SQInteger GuiWidgetDelegate::createPanel(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType));
        CHECK_FOR_WINDOW

        assert(parent->isWindow());
        GuiNamespace::createWidget(vm, parent, GuiNamespace::WidgetType::Panel);

        return 1;
    }

    SQInteger GuiWidgetDelegate::createSpinner(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType));
        CHECK_FOR_WINDOW

        assert(parent->isWindow());
        GuiNamespace::createWidget(vm, parent, GuiNamespace::WidgetType::Spinner);

        return 1;
    }

    SQInteger GuiWidgetDelegate::windowSizeScrollToFit(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType));
        CHECK_FOR_WINDOW

        assert(parent->isWindow());
        Colibri::Window* win = static_cast<Colibri::Window*>(parent);
        win->sizeScrollToFit();

        return 0;
    }

    SQInteger GuiWidgetDelegate::windowSetAllowMouseScroll(HSQUIRRELVM vm){
        SQBool allowMouseScroll = false;
        sq_getbool(vm, -1, &allowMouseScroll);

        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, -2, &parent, &foundType));
        //TODO should this and setMaxScroll return an error?
        CHECK_FOR_WINDOW

        assert(parent->isWindow());
        Colibri::Window* win = static_cast<Colibri::Window*>(parent);
        //TODO decide what to do about this.
        //win->setAllowsMouseGestureScroll(allowMouseScroll);

        return 0;
    }

    SQInteger GuiWidgetDelegate::windowSetConsumeCursor(HSQUIRRELVM vm){
        SQBool consumeCursor = false;
        sq_getbool(vm, -1, &consumeCursor);

        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, -2, &parent, &foundType));
        CHECK_FOR_WINDOW

        assert(parent->isWindow());
        Colibri::Window* win = static_cast<Colibri::Window*>(parent);
        win->setConsumeCursor(consumeCursor);

        return 0;
    }

    SQInteger GuiWidgetDelegate::windowSetMaxScroll(HSQUIRRELVM vm){
        SQFloat scrollX, scrollY;
        sq_getfloat(vm, 2, &scrollX);
        sq_getfloat(vm, 3, &scrollY);

        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType));
        CHECK_FOR_WINDOW

        assert(parent->isWindow());
        Colibri::Window* win = static_cast<Colibri::Window*>(parent);
        win->setMaxScroll(Ogre::Vector2(scrollX, scrollY));

        return 0;
    }

    SQInteger GuiWidgetDelegate::setClickable(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        assert(GuiNamespace::isTypeTagWidget(foundType));

        SQBool b;
        sq_getbool(vm, 2, &b);

        widget->setClickable(b);

        return 0;
    }

    SQInteger GuiWidgetDelegate::setKeyboardNavigable(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        assert(GuiNamespace::isTypeTagWidget(foundType));

        SQBool b;
        sq_getbool(vm, 2, &b);

        widget->setKeyboardNavigable(b);

        return 0;
    }

    SQInteger GuiWidgetDelegate::createWindow(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType));
        CHECK_FOR_WINDOW

        assert(parent->isWindow());
        Colibri::Window* window = dynamic_cast<Colibri::Window*>(parent);
        assert(window);

        GuiNamespace::createWindow(vm, window);

        return 1;
    }

    SQInteger _attachListener(HSQUIRRELVM vm, GuiNamespace::ActionType action){
        assert( (action >= Colibri::Action::Cancel && action <= Colibri::Action::ValueChanged) || action == GuiNamespace::ACTION_ANY);

        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        assert(GuiNamespace::isTypeTagBasicWidget(foundType));

        if(sq_gettype(vm, 2) != OT_CLOSURE) return sq_throwerror(vm, "AttachListener expects a closure object.");

        SQInteger top = sq_gettop(vm);

        SQObject targetFunction;
        sq_resetobject(&targetFunction);
        sq_getstackobj(vm, 2, &targetFunction);

        SQObject targetContext;
        sq_resetobject(&targetContext);
        if(top >= 3){
            sq_getstackobj(vm, 3, &targetContext);
        }

        SQInteger numParams, numFreeVariables;
        sq_getclosureinfo(vm, 2, &numParams, &numFreeVariables);
        if(numParams != 3) return sq_throwerror(vm, "Listener function must have arguments (widget, action)");

        GuiNamespace::WidgetType type = GuiNamespace::getWidgetTypeFromTypeTag(foundType);
        assert(type != GuiNamespace::WidgetType::Unknown);
        GuiNamespace::registerWidgetListener(widget, targetFunction, targetContext, type, action);

        return 0;
    }

    SQInteger GuiWidgetDelegate::attachListener(HSQUIRRELVM vm){
        return _attachListener(vm, GuiNamespace::ACTION_ANY);
    }

    SQInteger GuiWidgetDelegate::attachListenerForEvent(HSQUIRRELVM vm){
        ScriptUtils::_debugStack(vm);
        SQInteger action;
        sq_getinteger(vm, 3, &action);

        if(action < Colibri::Action::Cancel || action > Colibri::Action::ValueChanged){
            return sq_throwerror(vm, "Invalid action passed");
        }

        //Remove the integer to make it look like the regular attach listener.
        ScriptUtils::_debugStack(vm);
        if(sq_gettop(vm) >= 4){
            sq_remove(vm, 3);
        }else{
            sq_poptop(vm);
        }
        ScriptUtils::_debugStack(vm);

        return _attachListener(vm, static_cast<GuiNamespace::ActionType>(action));
    }

    SQInteger GuiWidgetDelegate::detachListener(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        CHECK_FOR_BASIC_WIDGET //Has to be a widget, but can't be a window.

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

    inline SQInteger labelFunction(HSQUIRRELVM vm, SQInteger idx, Colibri::Label** outLabel){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, idx, &widget, &foundType));

        if(foundType == WidgetButtonTypeTag){
            Colibri::Button* b = dynamic_cast<Colibri::Button*>(widget);
            assert(b);
            *outLabel = b->getLabel();
        }else if(foundType == WidgetLabelTypeTag || foundType == WidgetAnimatedLabelTypeTag){
            Colibri::Label* l = dynamic_cast<Colibri::Label*>(widget);
            assert(l);
            *outLabel = l;
        }else if(foundType == WidgetCheckboxTypeTag){
            Colibri::Checkbox* c = dynamic_cast<Colibri::Checkbox*>(widget);
            assert(c);
            *outLabel = c->getButton()->getLabel();
        }else if(foundType == WidgetEditboxTypeTag){
            Colibri::Editbox* e = dynamic_cast<Colibri::Editbox*>(widget);
            assert(e);
            *outLabel = e->getLabel();
        }else{
            return sq_throwerror(vm, "Invalid widget");
        }
        return 0;
    }

    SQInteger GuiWidgetDelegate::setDefaultFont(HSQUIRRELVM vm){
        SQInteger id;
        sq_getinteger(vm, 2, &id);

        Colibri::Label* l = 0;
        SQInteger result = labelFunction(vm, 1, &l);
        if(SQ_FAILED(result)) return result;

        //TODO would be nice to have a warning if the user has provided a bad font.
        l->setDefaultFont(static_cast<uint16_t>(id));

        return 0;
    }

    SQInteger GuiWidgetDelegate::setDefaultFontSize(HSQUIRRELVM vm){
        SQFloat size;
        sq_getfloat(vm, 2, &size);

        Colibri::Label* l = 0;
        SQInteger result = labelFunction(vm, 1, &l);
        if(SQ_FAILED(result)) return result;

        l->setDefaultFontSize(Colibri::FontSize(size));

        return 0;
    }

    SQInteger GuiWidgetDelegate::getDefaultFontSize(HSQUIRRELVM vm){
        Colibri::Label* l = 0;
        SQInteger result = labelFunction(vm, 1, &l);
        if(SQ_FAILED(result)) return result;

        Colibri::FontSize fSize = l->getDefaultFontSize();
        float size = fSize.asFloat();
        sq_pushfloat(vm, size);

        return 1;
    }

    SQInteger GuiWidgetDelegate::setTextHorizontalAlignment(HSQUIRRELVM vm){
        SQInteger alignType;
        sq_getinteger(vm, 2, &alignType);
        Colibri::TextHorizAlignment::TextHorizAlignment texAlignment = Colibri::TextHorizAlignment::Natural;
        switch(alignType){
            case 0: texAlignment = Colibri::TextHorizAlignment::Natural; break;
            case 1: texAlignment = Colibri::TextHorizAlignment::Left; break;
            case 2: texAlignment = Colibri::TextHorizAlignment::Center; break;
            case 3: texAlignment = Colibri::TextHorizAlignment::Right; break;
            default:{
                return sq_throwerror(vm, "Invalid alignment type");
                break;
            }
        }

        Colibri::Label* l = 0;
        SQInteger result = labelFunction(vm, 1, &l);
        if(SQ_FAILED(result)) return result;

        l->setTextHorizAlignment(texAlignment);

        return 0;
    }

    SQInteger GuiWidgetDelegate::setTextColour(HSQUIRRELVM vm){

        Colibri::Label* l = 0;
        SQInteger result = labelFunction(vm, 1, &l);
        if(SQ_FAILED(result)) return result;

        float r, g, b, a;
        a = 1.0f;
        sq_getfloat(vm, 2, &r);
        sq_getfloat(vm, 3, &g);
        sq_getfloat(vm, 4, &b);
        if(sq_gettop(vm) == 5) sq_getfloat(vm, 5, &a);

        l->setTextColour(Ogre::ColourValue(r, g, b, a));

        return 0;
    }

    SQInteger _parseRichTextTable(Colibri::RichText &text, HSQUIRRELVM vm){
        sq_pushnull(vm);
        while(SQ_SUCCEEDED(sq_next(vm, -2))){
            SQObjectType objectType = sq_gettype(vm, -1);

            const SQChar *key;
            sq_getstring(vm, -2, &key);

            SQObjectType t = sq_gettype(vm, -1);
            if(t == OT_INTEGER){
                SQInteger val;
                sq_getinteger(vm, -1, &val);

                if(strcmp(key, "offset") == 0){
                    text.offset = val;
                }
                else if(strcmp(key, "len") == 0){
                    text.length = val;
                }
                else if(strcmp(key, "font") == 0){
                    text.font = val;
                }
                else if(strcmp(key, "start") == 0){
                    text.glyphStart = val;
                }
                else if(strcmp(key, "end") == 0){
                    text.glyphEnd = val;
                }
                else return sq_throwerror(vm, "Invalid key in table.");
            }
            else if(t == OT_FLOAT){
                SQFloat val;
                sq_getfloat(vm, -1, &val);

                if(strcmp(key, "fontSize") == 0){ text.ptSize = Colibri::FontSize(val); }
                else return sq_throwerror(vm, "Invalid key in table.");
            }
            else if(t == OT_STRING){
                if(strcmp(key, "col") == 0){
                    const SQChar *value;
                    sq_getstring(vm, -1, &value);
                    Ogre::ColourValue val = Ogre::StringConverter::parseColourValue(value, Ogre::ColourValue::White);
                    text.rgba32 = val.getAsRGBA();
                }
                else return sq_throwerror(vm, "Invalid key in table.");
            }
            else return sq_throwerror(vm, "Invalid type in table.");

            sq_pop(vm, 2);
        }
        sq_pop(vm, 1);

        return 0;
    }

    SQInteger _parseRichTextArray(Colibri::RichTextVec &richText, const Colibri::RichText &defaultRichText, HSQUIRRELVM vm, SQInteger idx){
        sq_pushnull(vm);
        while(SQ_SUCCEEDED(sq_next(vm, idx))){
            SQObjectType objectType = sq_gettype(vm, -1);
            if(objectType != OT_TABLE){
                sq_pop(vm, 3);
                return sq_throwerror(vm, "Invalid type");
            }

            Colibri::RichText text = defaultRichText;
            SQInteger result = _parseRichTextTable(text, vm);
            if(result != 0) return result;
            richText.push_back(text);

            sq_pop(vm, 2);
        }
        sq_pop(vm, 1);

        return 0;
    }

    SQInteger GuiWidgetDelegate::setRichText(HSQUIRRELVM vm){

        Colibri::Label* l = 0;
        SQInteger result = labelFunction(vm, 1, &l);
        if(SQ_FAILED(result)) return result;

        Colibri::States::States targetState = Colibri::States::NumStates;
        SQInteger stackSize = sq_gettop(vm);
        if(stackSize >= 3){
            SQInteger stateType;
            sq_getinteger(vm, 3, &stateType);

            //-1 means any, everything else is mapped to the enums.
            if(stateType < -1 || stateType >= Colibri::States::NumStates) return sq_throwerror(vm, "Invalid state value.");
            if(stateType >= 0){
                targetState = static_cast<Colibri::States::States>(stateType);
            }
        }

        const Colibri::RichText defaultText = l->getDefaultRichText();
        Colibri::RichTextVec richText;
        SQInteger textResult = _parseRichTextArray(richText, defaultText, vm, 2);
        if(textResult != 0) return textResult;

        l->setRichText(richText, true, targetState);

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

    SQInteger GuiWidgetDelegate::setDatablock(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));

        SQObjectType t = sq_gettype(vm, 2);
        Colibri::Renderable* targetItem = dynamic_cast<Colibri::Renderable*>(widget);
        assert(targetItem);
        if(t == OT_USERDATA){
            Ogre::HlmsDatablock* db = 0;
            SCRIPT_CHECK_RESULT(DatablockUserData::getPtrFromUserData(vm, 2, &db));

            if(foundType == WidgetPanelTypeTag){
                Colibri::WrappedColibriRenderable* panel = dynamic_cast<Colibri::WrappedColibriRenderable*>(targetItem);
                assert(panel);
                panel->setDatablockAll(db->getName());
            }
            targetItem->setDatablock(db);
        }else if(t == OT_STRING){
            const SQChar *dbPath;
            sq_getstring(vm, 2, &dbPath);

            targetItem->setDatablock(dbPath);
            if(foundType == WidgetPanelTypeTag){
                Colibri::WrappedColibriRenderable* panel = dynamic_cast<Colibri::WrappedColibriRenderable*>(targetItem);
                assert(panel);
                panel->setDatablockAll(dbPath);
            }
        }else{
            assert(false);
        }

        return 0;
    }

    SQInteger GuiWidgetDelegate::getDatablock(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));

        Colibri::Renderable* targetItem = dynamic_cast<Colibri::Renderable*>(widget);
        assert(targetItem);
        Ogre::HlmsDatablock* db = targetItem->getDatablock();

        DatablockUserData::DatablockPtrToUserData(vm, db);
        return 1;
    }

    SQInteger GuiWidgetDelegate::setOrientation(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));

        SQFloat f;
        sq_getfloat(vm, 2, &f);

        widget->setOrientation(Ogre::Radian(Ogre::Real(f)));

        return 0;
    }

    SQInteger GuiWidgetDelegate::setVisualsEnabled(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));

        SQBool b;
        sq_getbool(vm, 2, &b);

        assert(widget->isRenderable());
        Colibri::Renderable* rend = dynamic_cast<Colibri::Renderable*>(widget);
        assert(rend);
        rend->setVisualsEnabled(b);

        return 0;
    }

    SQInteger GuiWidgetDelegate::setClipBorders(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));

        SQFloat top, left, right, bottom;
        sq_getfloat(vm, -4, &top);
        sq_getfloat(vm, -3, &left);
        sq_getfloat(vm, -2, &right);
        sq_getfloat(vm, -1, &bottom);

        float values[Colibri::Borders::NumBorders] = {top, left, right, bottom};
        widget->setClipBorders( values );

        return 0;
    }

    SQInteger GuiWidgetDelegate::setFocus(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));

        widget->setKeyboardFocus();

        return 0;
    }

    SQInteger GuiWidgetDelegate::setSliderRange(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));

        SQInteger min, max;
        sq_getinteger(vm, 2, &min);
        sq_getinteger(vm, 3, &max);

        Colibri::Slider* slider = (Colibri::Slider*)widget;

        slider->setRange(static_cast<uint32_t>(min), static_cast<uint32_t>(max));

        return 0;
    }

}
