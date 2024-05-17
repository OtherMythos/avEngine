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
#include "Scripting/ScriptNamespace/Classes/ColourValueUserData.h"

#include "OgreStringConverter.h"
#include "OgreRoot.h"
#include "OgreHlmsManager.h"

namespace AV{

    #define BASIC_WIDGET_FUNCTIONS \
        ScriptUtils::addFunction(vm, setPosition, "setPosition", -2, ".u|nn"); \
        ScriptUtils::addFunction(vm, setCentre, "setCentre", -2, ".u|nn"); \
        ScriptUtils::addFunction(vm, getPosition, "getPosition"); \
        ScriptUtils::addFunction(vm, getDerivedPosition, "getDerivedPosition"); \
        ScriptUtils::addFunction(vm, getDerivedCentre, "getDerivedCentre"); \
        ScriptUtils::addFunction(vm, getCentre, "getCentre"); \
        ScriptUtils::addFunction(vm, getSize, "getSize"); \
        ScriptUtils::addFunction(vm, getSizeAfterClipping, "getSizeAfterClipping"); \
        ScriptUtils::addFunction(vm, calculateChildrenSize, "calculateChildrenSize"); \
        ScriptUtils::addFunction(vm, setSize, "setSize", -2, ".u|nn"); \
        ScriptUtils::addFunction(vm, setHidden, "setHidden", 2, ".b"); \
        ScriptUtils::addFunction(vm, setVisible, "setVisible", 2, ".b"); \
        ScriptUtils::addFunction(vm, getVisible, "getVisible"); \
        ScriptUtils::addFunction(vm, setZOrder, "setZOrder", 2, ".i"); \
        ScriptUtils::addFunction(vm, setSkin, "setSkin", -2, ".si"); \
        ScriptUtils::addFunction(vm, setSkinPack, "setSkinPack", 2, ".s"); \
        ScriptUtils::addFunction(vm, setDatablock, "setDatablock", 2, ".u|s"); \
        ScriptUtils::addFunction(vm, setOrientation, "setOrientation", 2, ".f"); \
        ScriptUtils::addFunction(vm, getDatablock, "getDatablock"); \
        ScriptUtils::addFunction(vm, setClickable, "setClickable", 2, ".b"); \
        ScriptUtils::addFunction(vm, setKeyboardNavigable, "setKeyboardNavigable", 2, ".b"); \
        ScriptUtils::addFunction(vm, setFocus, "setFocus"); \
        ScriptUtils::addFunction(vm, setClipBorders, "setClipBorders", 5, ".nnnn"); \
        ScriptUtils::addFunction(vm, setVisualsEnabled, "setVisualsEnabled", 2, ".b"); \
        ScriptUtils::addFunction(vm, getType, "getType"); \
        \
        ScriptUtils::addFunction(vm, getWidgetUserId, "getUserId"); \
        ScriptUtils::addFunction(vm, setWidgetUserId, "setUserId", 2, ".i"); \
        \
        ScriptUtils::addFunction(vm, setExpandVertical, "setExpandVertical", 2, ".b"); \
        ScriptUtils::addFunction(vm, setExpandHorizontal, "setExpandHorizontal", 2, ".b"); \
        ScriptUtils::addFunction(vm, setProportionVertical, "setProportionVertical", 2, ".i"); \
        ScriptUtils::addFunction(vm, setProportionHorizontal, "setProportionHorizontal", 2, ".i"); \
        ScriptUtils::addFunction(vm, setPriority, "setPriority", 2, ".i"); \
        ScriptUtils::addFunction(vm, setGridLocation, "setGridLocation", 2, ".i"); \
        ScriptUtils::addFunction(vm, setMargin, "setMargin", -2, ".u|nn"); \
        ScriptUtils::addFunction(vm, setMinSize, "setMinSize", -2, ".u|nn"); \
        ScriptUtils::addFunction(vm, setDisabled, "setDisabled", 2, ".b"); \

    #define LISTENER_WIDGET_FUNCTIONS \
        ScriptUtils::addFunction(vm, attachListener, "attachListener", -2, ".ct|x"); \
        ScriptUtils::addFunction(vm, attachListenerForEvent, "attachListenerForEvent", -3, ".cit|x"); \
        ScriptUtils::addFunction(vm, detachListener, "detachListener");

    #define LABEL_WIDGET_FUNCTIONS \
        ScriptUtils::addFunction(vm, setDefaultFont, "setDefaultFont", 2, ".i"); \
        ScriptUtils::addFunction(vm, setDefaultFontSize, "setDefaultFontSize", 2, ".n"); \
        ScriptUtils::addFunction(vm, getDefaultFontSize, "getDefaultFontSize"); \
        ScriptUtils::addFunction(vm, setTextHorizontalAlignment, "setTextHorizontalAlignment", 2, ".i"); \
        ScriptUtils::addFunction(vm, setTextColour, "setTextColour", -2, ".u|nnnn"); \
        ScriptUtils::addFunction(vm, setRichText, "setRichText", -2, ".ai"); \
        ScriptUtils::addFunction(vm, sizeToFit, "sizeToFit", -1, ".n"); \
        ScriptUtils::addFunction(vm, setShadowOutline, "setShadowOutline", -2, ".buu");

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
        ScriptUtils::addFunction(vm, createWindow, "createWindow", -1, ".s");
        ScriptUtils::addFunction(vm, createPanel, "createPanel");
        ScriptUtils::addFunction(vm, createSpinner, "createSpinner");

        ScriptUtils::addFunction(vm, getNumChildren, "getNumChildren");
        ScriptUtils::addFunction(vm, getChildForIdx, "getChildForIdx");

        ScriptUtils::addFunction(vm, windowSizeScrollToFit, "sizeScrollToFit");
        ScriptUtils::addFunction(vm, windowSetMaxScroll, "setMaxScroll", -2, ".u|nn");
        ScriptUtils::addFunction(vm, windowSetAllowMouseScroll, "setAllowMouseScroll", 2, ".b");
        ScriptUtils::addFunction(vm, windowSetConsumeCursor, "setConsumeCursor", 2, ".b");
        ScriptUtils::addFunction(vm, windowGetCurrentScroll, "getCurrentScroll");
        ScriptUtils::addFunction(vm, windowSetColour, "setColour", 2, ".u");
        ScriptUtils::addFunction(vm, windowGetQueryName, "getQueryName");
    }

    void GuiWidgetDelegate::setupButton(HSQUIRRELVM vm){
        sq_newtableex(vm, 5);

        BASIC_WIDGET_FUNCTIONS
        LISTENER_WIDGET_FUNCTIONS
        LABEL_WIDGET_FUNCTIONS

        ScriptUtils::addFunction(vm, setText, "setText", -2, ".s|b");
        ScriptUtils::addFunction(vm, getText, "getText");

    }

    void GuiWidgetDelegate::setupLabel(HSQUIRRELVM vm){
        sq_newtableex(vm, 4);

        BASIC_WIDGET_FUNCTIONS
        LABEL_WIDGET_FUNCTIONS

        ScriptUtils::addFunction(vm, setText, "setText", -2, ".s|b");
        ScriptUtils::addFunction(vm, getText, "getText");
    }

    void GuiWidgetDelegate::setupAnimatedLabel(HSQUIRRELVM vm){
        sq_newtableex(vm, 4);

        BASIC_WIDGET_FUNCTIONS
        LABEL_WIDGET_FUNCTIONS

        ScriptUtils::addFunction(vm, setText, "setText", -2, ".s|b");
        ScriptUtils::addFunction(vm, getText, "getText");
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
        ScriptUtils::addFunction(vm, getText, "getText");

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
        ScriptUtils::addFunction(vm, getText, "getText");

        ScriptUtils::addFunction(vm, getSpinnerValue, "getValue");
        ScriptUtils::addFunction(vm, getSpinnerValueRaw, "getValueRaw");
        ScriptUtils::addFunction(vm, setSpinnerValueRaw, "setValueRaw");

        BASIC_WIDGET_FUNCTIONS
        LISTENER_WIDGET_FUNCTIONS
    }

    #undef BASIC_WIDGET_FUNCTIONS
    #undef LISTENER_WIDGET_FUNCTIONS
    #undef LABEL_WIDGET_FUNCTIONS

    inline SQInteger _labelFunction(HSQUIRRELVM vm, SQInteger idx, Colibri::Label** outLabel){
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
        }else if(foundType == WidgetSpinnerTypeTag){
            Colibri::Spinner* c = dynamic_cast<Colibri::Spinner*>(widget);
            assert(c);
            *outLabel = c->getLabel();
        }else if(foundType == WidgetAnimatedLabelTypeTag){
            AV::AnimatedLabel* c = dynamic_cast<AV::AnimatedLabel*>(widget);
            assert(c);
            *outLabel = c;
        }else{
            return sq_throwerror(vm, "Invalid widget");
        }
        return 0;
    }

    SQInteger GuiWidgetDelegate::getPosition(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        assert(GuiNamespace::isTypeTagWidget(foundType));

        Vector2UserData::vector2ToUserData(vm, widget->getLocalTopLeft());

        return 1;
    }

    SQInteger _getDerivedPosition(HSQUIRRELVM vm, bool centre){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        assert(GuiNamespace::isTypeTagWidget(foundType));

        Ogre::Vector2 derivedPos(0, 0);
        Colibri::Widget* w = widget->getParent();
        while(w != NULL){
            derivedPos += w->getLocalTopLeft();
            w = w->getParent();
        }
        derivedPos += (centre ? widget->getCenter() : widget->getLocalTopLeft());

        Vector2UserData::vector2ToUserData(vm, derivedPos);

        return 1;

    }
    SQInteger GuiWidgetDelegate::getDerivedPosition(HSQUIRRELVM vm){
        return _getDerivedPosition(vm, false);
    }
    SQInteger GuiWidgetDelegate::getDerivedCentre(HSQUIRRELVM vm){
        return _getDerivedPosition(vm, true);
    }

    SQInteger GuiWidgetDelegate::getSize(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        assert(GuiNamespace::isTypeTagWidget(foundType));

        Vector2UserData::vector2ToUserData(vm, widget->getSize());

        return 1;
    }

    SQInteger GuiWidgetDelegate::getSizeAfterClipping(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        assert(GuiNamespace::isTypeTagWidget(foundType));

        Vector2UserData::vector2ToUserData(vm, widget->getSizeAfterClipping());

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

    SQInteger GuiWidgetDelegate::getCentre(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        assert(GuiNamespace::isTypeTagWidget(foundType));

        Vector2UserData::vector2ToUserData(vm, widget->getCenter());

        return 1;
    }

    SQInteger GuiWidgetDelegate::setCentre(HSQUIRRELVM vm){
        Ogre::Vector2 outVec;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read2FloatsOrVec2(vm, &outVec));

        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        assert(GuiNamespace::isTypeTagWidget(foundType));

        widget->setCenter(outVec);

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
        sq_getbool(vm, 2, &value);

        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        assert(GuiNamespace::isTypeTagWidget(foundType));

        widget->setHidden(value);

        return 0;
    }

    SQInteger GuiWidgetDelegate::setVisible(HSQUIRRELVM vm){
        SQBool value;
        sq_getbool(vm, 2, &value);

        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        assert(GuiNamespace::isTypeTagWidget(foundType));

        widget->setHidden(!value);

        return 0;
    }

    SQInteger GuiWidgetDelegate::getVisible(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        assert(GuiNamespace::isTypeTagWidget(foundType));

        bool hidden = !(widget->isHidden());
        sq_pushbool(vm, hidden);

        return 1;
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

        //TODO try and merge this with to to label function to avoid duplication.
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

        Colibri::Label* l = 0;
        SQInteger result = _labelFunction(vm, 1, &l);
        if (SQ_FAILED(result)) return result;
        const char* str = l->getText().c_str();

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

        Colibri::Label* outLabel;
        SQInteger result = _labelFunction(vm, 1, &outLabel);
        if(SQ_FAILED(result)) return result;

        if(sq_gettop(vm) == 2){
            SQFloat maxWidth = 0.0f;
            sq_getfloat(vm, 2, &maxWidth);
            outLabel->sizeToFit(maxWidth);
        }else{
            outLabel->sizeToFit();
        }

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

    SQInteger GuiWidgetDelegate::windowGetCurrentScroll(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType));
        CHECK_FOR_WINDOW

        assert(parent->isWindow());
        Colibri::Window* win = static_cast<Colibri::Window*>(parent);
        const Ogre::Vector2 scroll = win->getCurrentScroll();

        Vector2UserData::vector2ToUserData(vm, scroll);

        return 1;
    }

    SQInteger GuiWidgetDelegate::windowSetColour(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType));
        CHECK_FOR_WINDOW

        assert(parent->isWindow());
        Colibri::Window* win = static_cast<Colibri::Window*>(parent);

        Ogre::ColourValue col;
        ColourValueUserData::readColourValueFromUserData(vm, 2, &col);

        win->setColour(true, col);

        return 0;
    }

    SQInteger GuiWidgetDelegate::windowGetQueryName(HSQUIRRELVM vm){
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType));
        CHECK_FOR_WINDOW

        assert(parent->isWindow());
        Colibri::Window* win = static_cast<Colibri::Window*>(parent);

        const std::string* outString = GuiNamespace::getQueryIdForWindow(win);
        sq_pushstring(vm, outString->c_str(), outString->length());

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
        Colibri::Widget* parent = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &parent, &foundType));
        CHECK_FOR_WINDOW

        Ogre::Vector2 outVec;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read2FloatsOrVec2(vm, &outVec));

        assert(parent->isWindow());
        Colibri::Window* win = static_cast<Colibri::Window*>(parent);
        win->setMaxScroll(outVec);

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

        std::string queryId = "";
        SQInteger value = sq_gettop(vm);
        if(value >= 2){
            const char* c = 0;
            sq_getstring(vm, 2, &c);
            queryId = c;
        }

        GuiNamespace::_createWindow(vm, window, queryId);

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
        SQInteger action;
        sq_getinteger(vm, 3, &action);

        if(action < Colibri::Action::Cancel || action > Colibri::Action::ValueChanged){
            return sq_throwerror(vm, "Invalid action passed");
        }

        //Remove the integer to make it look like the regular attach listener.
        if(sq_gettop(vm) >= 4){
            sq_remove(vm, 3);
        }else{
            sq_poptop(vm);
        }

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
        data->userIdx = static_cast<int>(id);

        return 0;
    }

    class WidgetDelegateExposure : public Colibri::Widget{
    public:
        size_t getNumWidgets() { return m_numWidgets - m_numNonRenderables; }
        Colibri::Widget* getChildForIdx(size_t idx) {
            size_t listIdx = m_numNonRenderables + idx;
            if(listIdx < 0 || listIdx >= m_children.size()) return 0;
            return m_children[listIdx];
        }
    };
    SQInteger GuiWidgetDelegate::getChildForIdx(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        assert(GuiNamespace::isTypeTagWidget(foundType));
        WidgetDelegateExposure* exposure = static_cast<WidgetDelegateExposure*>(widget);
        assert(exposure);

        SQInteger childIdx;
        sq_getinteger(vm, 2, &childIdx);

        Colibri::Widget* outWidget = exposure->getChildForIdx(childIdx);
        GuiNamespace::widgetToUserData(vm, outWidget);

        return 1;
    }
    SQInteger GuiWidgetDelegate::getNumChildren(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));
        assert(GuiNamespace::isTypeTagWidget(foundType));
        WidgetDelegateExposure* exposure = static_cast<WidgetDelegateExposure*>(widget);
        assert(exposure);

        size_t out = exposure->getNumWidgets();
        sq_pushinteger(vm, static_cast<SQInteger>(out));

        return 1;
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
        SQInteger id;
        sq_getinteger(vm, 2, &id);

        Colibri::Label* l = 0;
        SQInteger result = _labelFunction(vm, 1, &l);
        if(SQ_FAILED(result)) return result;

        //TODO would be nice to have a warning if the user has provided a bad font.
        l->setDefaultFont(static_cast<uint16_t>(id));

        return 0;
    }

    SQInteger GuiWidgetDelegate::setDefaultFontSize(HSQUIRRELVM vm){
        SQFloat size;
        sq_getfloat(vm, 2, &size);

        Colibri::Label* l = 0;
        SQInteger result = _labelFunction(vm, 1, &l);
        if(SQ_FAILED(result)) return result;

        l->setDefaultFontSize(Colibri::FontSize(size));

        return 0;
    }

    SQInteger GuiWidgetDelegate::setShadowOutline(HSQUIRRELVM vm){
        SQBool enable;
        Ogre::ColourValue colVal(Ogre::ColourValue::Black);
        Ogre::Vector2 displace(Ogre::Vector2::UNIT_SCALE);

        sq_getbool(vm, 2, &enable);
        if(sq_gettop(vm) >= 3){
            SCRIPT_CHECK_RESULT(ColourValueUserData::readColourValueFromUserData(vm, 3, &colVal));
        }
        if(sq_gettop(vm) >= 4){
            Vector2UserData::readVector2FromUserData(vm, 4, &displace);
        }

        Colibri::Label* l = 0;
        SQInteger result = _labelFunction(vm, 1, &l);
        if(SQ_FAILED(result)) return result;

        l->setShadowOutline(enable, colVal, displace);

        return 0;
    }

    SQInteger GuiWidgetDelegate::getDefaultFontSize(HSQUIRRELVM vm){
        Colibri::Label* l = 0;
        SQInteger result = _labelFunction(vm, 1, &l);
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
        SQInteger result = _labelFunction(vm, 1, &l);
        if(SQ_FAILED(result)) return result;

        l->setTextHorizAlignment(texAlignment);

        return 0;
    }

    SQInteger GuiWidgetDelegate::setTextColour(HSQUIRRELVM vm){

        Colibri::Label* l = 0;
        SQInteger result = _labelFunction(vm, 1, &l);
        if(SQ_FAILED(result)) return result;

        Ogre::ColourValue val(Ogre::ColourValue::White);
        if(sq_gettype(vm, 2) == OT_USERDATA){
            SCRIPT_CHECK_RESULT(ColourValueUserData::readColourValueFromUserData(vm, 2, &val));
        }else{
            float r, g, b, a;
            a = 1.0f;
            sq_getfloat(vm, 2, &r);
            sq_getfloat(vm, 3, &g);
            sq_getfloat(vm, 4, &b);
            if(sq_gettop(vm) == 5) sq_getfloat(vm, 5, &a);
            val = Ogre::ColourValue(r, g, b, a);
        }

        l->setTextColour(val);

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
                    text.offset = static_cast<uint32_t>(val);
                }
                else if(strcmp(key, "len") == 0){
                    text.length = static_cast<uint32_t>(val);
                }
                else if(strcmp(key, "font") == 0){
                    text.font = val;
                }
                else if(strcmp(key, "start") == 0){
                    text.glyphStart = static_cast<uint32_t>(val);
                }
                else if(strcmp(key, "end") == 0){
                    text.glyphEnd = static_cast<uint32_t>(val);
                }
                else if(strcmp(key, "col") == 0){
                    text.rgba32 = static_cast<uint32_t>(val);
                }
                else return sq_throwerror(vm, "Invalid key in table.");
            }
            else if(t == OT_FLOAT){
                SQFloat val;
                sq_getfloat(vm, -1, &val);

                if(strcmp(key, "fontSize") == 0){ text.ptSize = Colibri::FontSize(val); }
                else return sq_throwerror(vm, "Invalid key in table.");
            }
            else if(t == OT_USERDATA){
                if(strcmp(key, "col") == 0){
                    Ogre::ColourValue colValue;
                    ColourValueUserData::readColourValueFromUserData(vm, -1, &colValue);
                    text.rgba32 = colValue.getAsABGR();
                }
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
        SQInteger result = _labelFunction(vm, 1, &l);
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

            if(db->mType != Ogre::HLMS_UNLIT){
                return sq_throwerror(vm, "Datablocks for panels must be unlit type.");
            }

            if(foundType == WidgetPanelTypeTag){
                Colibri::WrappedColibriRenderable* panel = dynamic_cast<Colibri::WrappedColibriRenderable*>(targetItem);
                assert(panel);
                panel->setDatablockAll(db->getName());
            }
            targetItem->setDatablock(db);
        }else if(t == OT_STRING){
            const SQChar *dbPath;
            sq_getstring(vm, 2, &dbPath);

            Ogre::HlmsManager *hlmsManager = Ogre::Root::getSingleton().getHlmsManager();
            Ogre::HlmsDatablock *retVal = hlmsManager->getDatablockNoDefault(dbPath);

            if(!retVal){
                return sq_throwerror(vm, (Ogre::String("Could not find datablock with name ") + dbPath).c_str());
            }
            if(retVal->mType != Ogre::HLMS_UNLIT){
                return sq_throwerror(vm, "Datablocks for panels must be unlit type.");
            }

            targetItem->setDatablock(retVal);
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

    SQInteger GuiWidgetDelegate::getType(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));

        GuiNamespace::GuiWidgetUserData* outData;
        bool data = GuiNamespace::getWidgetData(widget, &outData);

        GuiNamespace::WidgetType widgetType = outData->type;

        sq_pushinteger(vm, (SQInteger)widgetType);

        return 1;
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

    SQInteger GuiWidgetDelegate::setExpandVertical(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));

        SQBool expand;
        sq_getbool(vm, 2, &expand);

        widget->m_expand[1] = expand;

        return 0;
    }

    SQInteger GuiWidgetDelegate::setExpandHorizontal(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));

        SQBool expand;
        sq_getbool(vm, 2, &expand);

        widget->m_expand[0] = expand;

        return 0;
    }

    SQInteger GuiWidgetDelegate::setProportionVertical(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));

        SQInteger proportion;
        sq_getinteger(vm, 2, &proportion);
        if(proportion < 0 || proportion >= 0xFFFF) return sq_throwerror(vm, "Proportion must be between 0 and 65535.");

        widget->m_proportion[1] = static_cast<uint16_t>(proportion);

        return 0;
    }

    SQInteger GuiWidgetDelegate::setProportionHorizontal(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));

        SQInteger proportion;
        sq_getinteger(vm, 2, &proportion);
        if(proportion < 0 || proportion >= 0xFFFF) return sq_throwerror(vm, "Proportion must be between 0 and 65535.");

        widget->m_proportion[0] = static_cast<uint16_t>(proportion);

        return 0;
    }

    SQInteger GuiWidgetDelegate::setPriority(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));

        SQInteger priority;
        sq_getinteger(vm, 2, &priority);
        if(priority < 0 || priority > 0xFF) return sq_throwerror(vm, "Priority must be between 0 and 255.");

        widget->m_priority = static_cast<uint8_t>(priority);

        return 0;
    }

    SQInteger GuiWidgetDelegate::setMargin(HSQUIRRELVM vm){
        Ogre::Vector2 outVec;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read2FloatsOrVec2(vm, &outVec));

        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));

        widget->m_margin = outVec;

        return 0;
    }

    SQInteger GuiWidgetDelegate::setMinSize(HSQUIRRELVM vm){
        Ogre::Vector2 outVec;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read2FloatsOrVec2(vm, &outVec));

        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));

        widget->m_minSize = outVec;

        return 0;
    }

    SQInteger GuiWidgetDelegate::setDisabled(HSQUIRRELVM vm){
        SQBool disabled;
        sq_getbool(vm, -1, &disabled);

        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));

        widget->setState(disabled ? Colibri::States::Disabled : Colibri::States::Idle);

        return 0;
    }

    SQInteger GuiWidgetDelegate::calculateChildrenSize(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));

        const Ogre::Vector2 childrenSize = widget->calculateChildrenSize();
        Vector2UserData::vector2ToUserData(vm, childrenSize + widget->getBorderCombined());

        return 1;
    }

    SQInteger GuiWidgetDelegate::setGridLocation(HSQUIRRELVM vm){
        Colibri::Widget* widget = 0;
        void* foundType = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getWidgetFromUserData(vm, 1, &widget, &foundType));

        SQInteger targetLocation;
        sq_getinteger(vm, 2, &targetLocation);

        if(targetLocation < 0 || targetLocation >= Colibri::GridLocations::NumGridLocations){
            return sq_throwerror(vm, "Invalid grid location provided.");
        }
        Colibri::GridLocations::GridLocations loc = static_cast<Colibri::GridLocations::GridLocations>(targetLocation);

        widget->m_gridLocation = loc;

        return 0;
    }
}
