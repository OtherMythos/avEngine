#include "GuiNamespace.h"

#include "System/BaseSingleton.h"

#include "ColibriGui/ColibriManager.h"
#include "Gui/GuiManager.h"
#include "Scripting/ScriptObjectTypeTags.h"
#include "Scripting/ScriptNamespace/Classes/Gui/GuiWidgetDelegate.h"
#include "Scripting/ScriptNamespace/Classes/Gui/GuiSizerDelegate.h"
#include "Scripting/ScriptNamespace/Classes/Vector2UserData.h"
#include "System/Util/PathUtils.h"

#include "ColibriGui/ColibriWindow.h"
#include "ColibriGui/ColibriButton.h"
#include "ColibriGui/ColibriLabel.h"
#include "ColibriGui/ColibriEditbox.h"
#include "ColibriGui/ColibriSlider.h"
#include "ColibriGui/ColibriCheckbox.h"
#include "ColibriGui/ColibriSpinner.h"
#include "ColibriGui/Layouts/ColibriLayoutLine.h"
#include "Gui/AnimatedLabel.h"

#include "Window/Window.h"
#include "Window/InputMapper.h"

#include "Gui/WrappedColibriRenderable.h"

#include <vector>
#include <map>

namespace AV{

    class GuiNamespaceWidgetListener : public Colibri::WidgetListener{
    public:
        void notifyWidgetDestroyed(Colibri::Widget *widget){
            GuiNamespace::_notifyWidgetDestruction(widget);
        }
    };

    class GuiNamespaceWidgetActionListener : public Colibri::WidgetActionListener{
    public:
        void notifyWidgetAction(Colibri::Widget *widget, Colibri::Action::Action action){
            GuiNamespace::_notifyWidgetActionPerformed(widget, action);
        }
    };

    //A list of all the windows ever created, unless they've been properly destroyed. This is used for correct destruction of objects during engine shutdown.
    static std::vector<Colibri::Window*> _createdWindows;
    static std::vector<Colibri::Widget*> _storedPointers;
    static std::vector<GuiNamespace::WidgetVersion> _storedVersions;
    static std::vector<GuiNamespace::GuiWidgetUserData> _storedWidgetUserData;

    static SQObject windowDelegateTable;
    static SQObject buttonDelegateTable;
    static SQObject labelDelegateTable;
    static SQObject editboxDelegateTable;
    static SQObject sliderDelegateTable;
    static SQObject checkboxDelegateTable;
    static SQObject panelDelegateTable;
    static SQObject spinnerDelegateTable;
    static SQObject animatedLabelDelegateTable;

    static SQObject sizerLayoutLineDelegateTable;

    static GuiNamespaceWidgetListener mNamespaceWidgetListener;
    static GuiNamespaceWidgetActionListener mNamespaceWidgetActionListener;

    //typedef std::pair<SQObject, GuiNamespace::WidgetType> ListenerFunction;
    struct ListenerFunction{
        SQObject first;
        GuiNamespace::WidgetType second;
        SQObject context;
    };
    static std::map<GuiNamespace::WidgetId, ListenerFunction> _attachedListeners;
    static HSQUIRRELVM _vm; //A static reference to the vm for the action callback functions.

    static const uint32_t _listenerMask = Colibri::ActionMask::Cancel | Colibri::ActionMask::Highlighted | Colibri::ActionMask::Hold | Colibri::ActionMask::PrimaryActionPerform | Colibri::ActionMask::SecondaryActionPerform | Colibri::ActionMask::ValueChanged;

    int GuiNamespace::getNumWindows(){
        return _createdWindows.size();
    }

    int GuiNamespace::getNumWidgets(){
        int count = 0;

        for(Colibri::Widget* w : _storedPointers){
            if(!w) continue;

            if(!w->isWindow()) count++;
        }

        return count;
    }

    SQInteger GuiNamespace::createWindow(HSQUIRRELVM vm, Colibri::Window* parentWindow){
        Colibri::Window* win = BaseSingleton::getGuiManager()->getColibriManager()->createWindow(parentWindow);
        //By default disable scroll.
        win->setMaxScroll(Ogre::Vector2::ZERO);

        WidgetId id = _storeWidget(win);
        win->m_userId = id;
        win->addListener(&mNamespaceWidgetListener);
        _widgetIdToUserData(vm, id, WidgetWindowTypeTag);
        _createdWindows.push_back(win);

        sq_pushobject(vm, windowDelegateTable);

        sq_setdelegate(vm, -2);

        return 1;
    }

    SQInteger GuiNamespace::createWindow(HSQUIRRELVM vm){
        Colibri::Window* parent = 0;
        SQInteger topIdx = sq_gettop(vm);
        if(topIdx == 2){
            void* expectedType;
            Colibri::Widget* outWidget = 0;
            SCRIPT_CHECK_RESULT(getWidgetFromUserData(vm, -1, &outWidget, &expectedType));
            if(expectedType != WidgetWindowTypeTag) return sq_throwerror(vm, "Only a window as a parent can be provided.");
            parent = dynamic_cast<Colibri::Window*>(outWidget);
            assert(parent);
        }

        return createWindow(vm, parent);
    }

    SQInteger GuiNamespace::createLayoutLine(HSQUIRRELVM vm){
        //They're just created on the heap for now. There's no tracking of these pointers in this class.
        //When the object goes out of scope the release hook destroys it.
        Colibri::LayoutLine* line = new Colibri::LayoutLine(BaseSingleton::getGuiManager()->getColibriManager());

        SQInteger stackSize = sq_gettop(vm);
        SQInteger layoutType = 0;
        if(stackSize >= 2){
            sq_getinteger(vm, 2, &layoutType);
            line->m_vertical = layoutType == 0;
        }

        Colibri::LayoutBase** pointer = (Colibri::LayoutBase**)sq_newuserdata(vm, sizeof(Colibri::LayoutBase*));
        *pointer = line;

        sq_setreleasehook(vm, -1, layoutReleaseHook);
        sq_settypetag(vm, -1, LayoutLineTypeTag);

        sq_pushobject(vm, sizerLayoutLineDelegateTable);

        sq_setdelegate(vm, -2);

        return 1;
    }

    SQInteger GuiNamespace::destroyWidget(HSQUIRRELVM vm){
        void* expectedType;
        Colibri::Widget* outWidget = 0;
        SCRIPT_CHECK_RESULT(getWidgetFromUserData(vm, -1, &outWidget, &expectedType));
        if(!isTypeTagWidget(expectedType)) return sq_throwerror(vm, "Incorrect object type passed");
        if(!outWidget) return sq_throwerror(vm, "Object handle is invalid.");

        //The widget listener will make the call to unstore the widget.
        BaseSingleton::getGuiManager()->getColibriManager()->destroyWidget(outWidget);

        return 0;
    }

    SQInteger GuiNamespace::loadSkins(HSQUIRRELVM vm){
        const SQChar *path;
        sq_getstring(vm, -1, &path);

        std::string outString;
        formatResToPath(path, outString);

        BaseSingleton::getGuiManager()->getColibriManager()->loadSkins(outString.c_str());

        return 0;
    }

    SQInteger GuiNamespace::setCanvasSize(HSQUIRRELVM vm){
        Ogre::Vector2 pointSize;
        SCRIPT_CHECK_RESULT(Vector2UserData::readVector2FromUserData(vm, 2, &pointSize));
        Ogre::Vector2 windowResolution;
        SCRIPT_CHECK_RESULT(Vector2UserData::readVector2FromUserData(vm, 3, &windowResolution));

        BaseSingleton::getGuiManager()->getColibriManager()->setCanvasSize(pointSize, windowResolution);

        return 0;
    }

    SQInteger GuiNamespace::setScrollSpeed(HSQUIRRELVM vm){
        SQFloat scrollSpeed;
        sq_getfloat(vm, -1, &scrollSpeed);

        BaseSingleton::getGuiManager()->setMouseScrollSpeed(scrollSpeed);

        return 0;
    }


    void GuiNamespace::setupNamespace(HSQUIRRELVM vm){
        ScriptUtils::setupDelegateTable(vm, &windowDelegateTable, GuiWidgetDelegate::setupWindow);
        ScriptUtils::setupDelegateTable(vm, &buttonDelegateTable, GuiWidgetDelegate::setupButton);
        ScriptUtils::setupDelegateTable(vm, &labelDelegateTable, GuiWidgetDelegate::setupLabel);
        ScriptUtils::setupDelegateTable(vm, &editboxDelegateTable, GuiWidgetDelegate::setupEditbox);
        ScriptUtils::setupDelegateTable(vm, &sliderDelegateTable, GuiWidgetDelegate::setupSlider);
        ScriptUtils::setupDelegateTable(vm, &checkboxDelegateTable, GuiWidgetDelegate::setupCheckbox);
        ScriptUtils::setupDelegateTable(vm, &panelDelegateTable, GuiWidgetDelegate::setupPanel);
        ScriptUtils::setupDelegateTable(vm, &spinnerDelegateTable, GuiWidgetDelegate::setupSpinner);
        ScriptUtils::setupDelegateTable(vm, &animatedLabelDelegateTable, GuiWidgetDelegate::setupAnimatedLabel);

        ScriptUtils::setupDelegateTable(vm, &sizerLayoutLineDelegateTable, GuiSizerDelegate::setupLayoutLine);

        /**SQNamespace
        @name _gui
        @desc A namespace to create and control gui elements.
        */

        /**SQFunction
        @name createWindow
        @desc Create a window.
        @param1:Window:Parent window.
        @returns A window object.
        */
        ScriptUtils::addFunction(vm, createWindow, "createWindow", -1, ".u");
        /**SQFunction
        @name createLayoutLine
        @desc Create a layout line object.
        @param1:alignment:An iterator specifying whether this is a horizontal or vertical layout line.
        @returns A layout line object.
        */
        ScriptUtils::addFunction(vm, createLayoutLine, "createLayoutLine", -1, ".i");
        /**SQFunction
        @name destroy
        @desc Destroy a gui element.
        @param1:GuiElement:A widget, window or layout object.
        */
        ScriptUtils::addFunction(vm, destroyWidget, "destroy", 2, ".u");
        /**SQFunction
        @name loadSkins
        @desc Load a GUI skin file.
        @param1:String:A res path to the target skin file.
        */
        ScriptUtils::addFunction(vm, loadSkins, "loadSkins", 2, ".s");

        /**SQFunction
        @name setCanvasSize
        @desc Set the size of the canvas.
        @param1:Vec2:The size of the canvas in points.
        @param2:Vec2:The window resolution. Does not necessarily have to match the canvas size.
        */
        ScriptUtils::addFunction(vm, setCanvasSize, "setCanvasSize", 3, ".uu");

         /**SQFunction
        @name setScrollSpeed
        @desc Set the speed at which the scroll wheel operates.
        @param1:Vec2:The size of the canvas in points.
        */
        ScriptUtils::addFunction(vm, setScrollSpeed, "setScrollSpeed", 2, ".n");


        /**SQFunction
        @name mapControllerInput
        @desc Map a controller input to a gui input.
        */
        ScriptUtils::addFunction(vm, mapControllerInput, "mapControllerInput", 3, ".ii");
        /**SQFunction
        @name mapControllerInput
        @desc Map a controller input to a gui input.
        */
        ScriptUtils::addFunction(vm, mapKeyboardInput, "mapKeyboardInput", 3, ".ii");
        /**SQFunction
        @name mapControllerAxis
        @desc Map a controller axis to four inputs.
        */
        ScriptUtils::addFunction(vm, mapControllerAxis, "mapControllerAxis", 6, ".iiiii");
        /**SQFunction
        @name getMousePosGui
        @desc Get the mouse position relative to the gui system.
        */
        ScriptUtils::addFunction(vm, getMousePosGui, "getMousePosGui");

        _vm = vm;
    }

    void GuiNamespace::setupConstants(HSQUIRRELVM vm){
        /**SQConstant
        @name _GUI_WIDGET_STATE_DISABLED
        @desc GUI state disabled.
        */
        ScriptUtils::declareConstant(vm, "_GUI_WIDGET_STATE_DISABLED", Colibri::States::Disabled);
        /**SQConstant
        @name _GUI_WIDGET_STATE_IDLE
        @desc GUI state idle.
        */
        ScriptUtils::declareConstant(vm, "_GUI_WIDGET_STATE_IDLE", Colibri::States::Idle);
        /**SQConstant
        @name _GUI_WIDGET_STATE_HIGHLIGHTED_CURSOR
        @desc GUI state highlighted by cursor.
        */
        ScriptUtils::declareConstant(vm, "_GUI_WIDGET_STATE_HIGHLIGHTED_CURSOR", Colibri::States::HighlightedCursor);
        /**SQConstant
        @name _GUI_WIDGET_STATE_HIGHLIGHTED_BUTTON
        @desc GUI state highlighted by keyboard or physical input device.
        */
        ScriptUtils::declareConstant(vm, "_GUI_WIDGET_STATE_HIGHLIGHTED_BUTTON", Colibri::States::HighlightedButton);
        /**SQConstant
        @name _GUI_WIDGET_STATE_HIGHLIGHTED_CURSOR_BUTTON
        @desc GUI state highlighted by cursor or button.
        */
        ScriptUtils::declareConstant(vm, "_GUI_WIDGET_STATE_HIGHLIGHTED_CURSOR_BUTTON", Colibri::States::HighlightedButtonAndCursor);
        /**SQConstant
        @name _GUI_WIDGET_STATE_PRESSED
        @desc GUI state pressed.
        */
        ScriptUtils::declareConstant(vm, "_GUI_WIDGET_STATE_PRESSED", Colibri::States::Pressed);


        /**SQConstant
        @name _GUI_INPUT_NONE
        @desc Unset input type. Use this if unsetting a gui input map.
        */
        ScriptUtils::declareConstant(vm, "_GUI_INPUT_NONE", static_cast<SQInteger>(GuiInputTypes::None));
        /**SQConstant
        @name _GUI_INPUT_TOP
        @desc Gui input border top/up.
        */
        ScriptUtils::declareConstant(vm, "_GUI_INPUT_TOP", static_cast<SQInteger>(GuiInputTypes::Top));
        /**SQConstant
        @name _GUI_INPUT_BOTTOM
        @desc Gui input border bottom/down.
        */
        ScriptUtils::declareConstant(vm, "_GUI_INPUT_BOTTOM", static_cast<SQInteger>(GuiInputTypes::Bottom));
        /**SQConstant
        @name _GUI_INPUT_LEFT
        @desc Gui input border left.
        */
        ScriptUtils::declareConstant(vm, "_GUI_INPUT_LEFT", static_cast<SQInteger>(GuiInputTypes::Left));
        /**SQConstant
        @name _GUI_INPUT_RIGHT
        @desc Gui input border right.
        */
        ScriptUtils::declareConstant(vm, "_GUI_INPUT_RIGHT", static_cast<SQInteger>(GuiInputTypes::Right));
        /**SQConstant
        @name _GUI_INPUT_PRIMARY
        @desc Primary input represents accepting the current input. For instance if a button is highlighted primary input would be selecting it.
        */
        ScriptUtils::declareConstant(vm, "_GUI_INPUT_PRIMARY", static_cast<SQInteger>(GuiInputTypes::Primary));
    }

    void GuiNamespace::_notifyWidgetDestruction(Colibri::Widget* widget){
        WidgetId id = widget->m_userId;
        _unstoreWidget(id);


        unbindWidgetListener(widget);

        if(widget->isWindow()){
            auto it = std::find(_createdWindows.begin(), _createdWindows.end(), widget);
            if(it != _createdWindows.end()){
                _createdWindows.erase(it);
            }
        }
    }

    void GuiNamespace::unbindWidgetListener(Colibri::Widget* widget){
        WidgetId id = widget->m_userId;

        auto it = _attachedListeners.find(id);
        if(it != _attachedListeners.end()){
            SQObject obj = (*it).second.first;
            SQObject targetContext = (*it).second.context;
            sq_release(_vm, &obj);
            sq_release(_vm, &targetContext);

            _attachedListeners.erase(it);
            widget->removeActionListener(&mNamespaceWidgetActionListener, _listenerMask);
        }
    }

    void GuiNamespace::registerWidgetListener(Colibri::Widget* widget, SQObject targetFunction, SQObject targetContext, WidgetType type){
        WidgetId id = widget->m_userId;
        if(!_isWidgetIdValid(id)) return;

        //It's now confirmed for storage, so increase the reference so it's not destroyed until its released.
        sq_addref(_vm, &targetFunction);
        if(targetContext._type != OT_NULL) sq_addref(_vm, &targetContext);
        _attachedListeners[id] = {targetFunction, type, targetContext};

        widget->addActionListener(&mNamespaceWidgetActionListener, _listenerMask);
    }

    void GuiNamespace::_notifyWidgetActionPerformed(Colibri::Widget* widget, Colibri::Action::Action action){
        WidgetId id = widget->m_userId;
        auto it = _attachedListeners.find(id);
        if(it == _attachedListeners.end()) return;

        //Determine the type tag and delegate table to use.
        WidgetType type = (*it).second.second;
        void* typeTag = 0;
        SQObject* delegateTable = 0;
        switch(type){
            case WidgetType::Editbox:
                typeTag = WidgetEditboxTypeTag;
                delegateTable = &editboxDelegateTable;
                break;
            case WidgetType::Button:
                typeTag = WidgetButtonTypeTag;
                delegateTable = &buttonDelegateTable;
                break;
            case WidgetType::Slider:
                typeTag = WidgetSliderTypeTag;
                delegateTable = &sliderDelegateTable;
                break;
            case WidgetType::Checkbox:
                typeTag = WidgetCheckboxTypeTag;
                delegateTable = &checkboxDelegateTable;
                break;
            case WidgetType::Panel:
                typeTag = WidgetPanelTypeTag;
                delegateTable = &panelDelegateTable;
                break;
            case WidgetType::Spinner:
                typeTag = WidgetSpinnerTypeTag;
                delegateTable = &spinnerDelegateTable;
                break;
            default:
                assert(false);
                break;
        }

        assert(typeTag);
        assert(delegateTable);

        sq_pushobject(_vm, (*it).second.first);

        SQObject targetContext = (*it).second.context;
        if(targetContext._type == OT_NULL){
            sq_pushroottable(_vm);
        }else{
            sq_pushobject(_vm, targetContext);
        }

        _widgetIdToUserData(_vm, id, typeTag); //TODO I should not be assuming what type it is.
        sq_pushobject(_vm, *delegateTable); //I'd like to move this somewhere else as well.
        sq_setdelegate(_vm, -2);

        sq_pushinteger(_vm, (SQInteger)action);

        SQInteger paramCount = 3;

        if(SQ_FAILED(sq_call(_vm, paramCount, false, true))){
            //return false;
        }
    }

    void GuiNamespace::destroyStoredWidgets(){
        Colibri::ColibriManager* man = BaseSingleton::getGuiManager()->getColibriManager();
        for(Colibri::Window* w : _createdWindows){
            if(!w) continue;
            //This function calls delete on the pointer, as well as all its children.
            man->destroyWindow(w);
        }
        _createdWindows.clear();
        _storedPointers.clear();
        _storedVersions.clear();
        _storedWidgetUserData.clear();
    }

    UserDataGetResult GuiNamespace::getLayoutFromUserData(HSQUIRRELVM vm, SQInteger idx, Colibri::LayoutBase** outValue){
        SQUserPointer pointer, typeTag;
        if(!SQ_SUCCEEDED(sq_getuserdata(vm, idx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != LayoutLineTypeTag){
            *outValue = 0;
            return USER_DATA_GET_INCORRECT_TYPE;
        }
        assert(pointer);

        Colibri::LayoutBase** p = static_cast<Colibri::LayoutBase**>(pointer);
        *outValue = *p;

        return USER_DATA_GET_SUCCESS;
    }

    UserDataGetResult GuiNamespace::getWidgetFromUserData(HSQUIRRELVM vm, SQInteger idx, Colibri::Widget** outValue, void** expectedType){
        WidgetId outId;
        UserDataGetResult result = _widgetIdFromUserData(vm, idx, &outId, expectedType);
        if(result != USER_DATA_GET_SUCCESS) return result;

        *outValue = GuiNamespace::_getWidget(outId);

        return result;
    }

    bool GuiNamespace::isTypeTagBasicWidget(void* tag){
        return
            tag == WidgetButtonTypeTag ||
            tag == WidgetLabelTypeTag ||
            tag == WidgetEditboxTypeTag ||
            tag == WidgetSliderTypeTag ||
            tag == WidgetCheckboxTypeTag ||
            tag == WidgetPanelTypeTag ||
            tag == WidgetAnimatedLabelTypeTag ||
            tag == WidgetSpinnerTypeTag
            ;
    }

    bool GuiNamespace::isTypeTagWidget(void* tag){
        return
            isTypeTagBasicWidget(tag) ||
            tag == WidgetWindowTypeTag
            ;
    }

    void GuiNamespace::createWidget(HSQUIRRELVM vm, Colibri::Widget* parentWidget, WidgetType type){
        Colibri::Widget* w = 0;
        Colibri::ColibriManager* man = BaseSingleton::getGuiManager()->getColibriManager();
        SQObject* targetTable = 0;
        void* typeTag = 0;
        switch(type){
            case WidgetType::Button:
                w = man->createWidget<Colibri::Button>(parentWidget);
                targetTable = &buttonDelegateTable;
                typeTag = WidgetButtonTypeTag;
                break;
            case WidgetType::Label:
                w = man->createWidget<Colibri::Label>(parentWidget);
                targetTable = &labelDelegateTable;
                typeTag = WidgetLabelTypeTag;
                break;
            case WidgetType::AnimatedLabel:
                w = man->createWidget<AnimatedLabel>(parentWidget);
                targetTable = &animatedLabelDelegateTable;
                typeTag = WidgetAnimatedLabelTypeTag;
                break;
            case WidgetType::Editbox:
                w = man->createWidget<Colibri::Editbox>(parentWidget);
                targetTable = &editboxDelegateTable;
                typeTag = WidgetEditboxTypeTag;
                break;
            case WidgetType::Slider:
                w = man->createWidget<Colibri::Slider>(parentWidget);
                targetTable = &sliderDelegateTable;
                typeTag = WidgetSliderTypeTag;
                break;
            case WidgetType::Checkbox:
                w = man->createWidget<Colibri::Checkbox>(parentWidget);
                targetTable = &checkboxDelegateTable;
                typeTag = WidgetCheckboxTypeTag;
                break;
            case WidgetType::Panel:{
                w = man->createWidget<Colibri::WrappedColibriRenderable>(parentWidget);
                targetTable = &panelDelegateTable;
                typeTag = WidgetPanelTypeTag;
                Colibri::Renderable* thingRend = dynamic_cast<Colibri::Renderable*>(w);
                //Populates with the correct uvs mainly.
                thingRend->setSkinPack("internal/PanelSkin");
                w->setPressable(true);
                w->setClickable(true);
                w->setKeyboardNavigable(true);
                break;
            }
            case WidgetType::Spinner:
                w = man->createWidget<Colibri::Spinner>(parentWidget);
                targetTable = &spinnerDelegateTable;
                typeTag = WidgetSpinnerTypeTag;
                break;
            default:
                assert(false);
                break;
        }

        WidgetId id = _storeWidget(w);
        w->m_userId = id;
        //OPTIMISATION Each widget seems to contain a vector for listeners, however I only need the one.
        //It seems to me like modifications to the library might help fix this.
        w->addListener(&mNamespaceWidgetListener);
        _widgetIdToUserData(vm, id, typeTag);

        sq_pushobject(vm, *targetTable);
        sq_setdelegate(vm, -2);
    }

    SQInteger GuiNamespace::widgetReleaseHook(SQUserPointer p, SQInteger size){

        //Read the id from the user data and remove it from the lists.
        WidgetId* id = (WidgetId*)p;
        //TODO update if this function isn't necessary.
        //_unstoreWidget(*id); //Don't unstore it over this. The widget is removed when it is destroyed.

        return 0;
    }

    SQInteger GuiNamespace::layoutReleaseHook(SQUserPointer p, SQInteger size){

        Colibri::LayoutBase** pointer = static_cast<Colibri::LayoutBase**>(p);
        delete *pointer;

        return 0;
    }

    void GuiNamespace::_widgetIdToUserData(HSQUIRRELVM vm, WidgetId id, void* typeTag){
        WidgetId* pointer = (WidgetId*)sq_newuserdata(vm, sizeof(WidgetId));
        *pointer = id;

        sq_setreleasehook(vm, -1, widgetReleaseHook);
        sq_settypetag(vm, -1, typeTag);
    }

    UserDataGetResult GuiNamespace::_widgetIdFromUserData(HSQUIRRELVM vm, SQInteger idx, WidgetId* outId, void** outTypeTag){
        SQUserPointer pointer, typeTag;
        if(!SQ_SUCCEEDED(sq_getuserdata(vm, idx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        *outTypeTag = typeTag;
        assert(pointer);

        WidgetId* p = static_cast<WidgetId*>(pointer);
        *outId = *p;

        return USER_DATA_GET_SUCCESS;
    }

    GuiNamespace::WidgetId GuiNamespace::_storeWidget(Colibri::Widget* widget){
        assert(_storedVersions.size() == _storedPointers.size()); //Each pointer should have a version

        //OPTIMISATION rather than searching the entire list linearly I could have some sort of skip list.
        for(uint32_t i = 0; i < _storedPointers.size(); i++){
            if(_storedPointers[i] != 0x0) continue;

            //An empty entry was found.
            _storedPointers[i] = widget;
            //I don't increase the version here. That's done during the deletion.

            return _produceWidgetId(i, _storedVersions[i]);
        }

        //No space in the list was found, so just create one.
        uint32_t idx = _storedPointers.size();
        _storedPointers.push_back(widget);
        _storedVersions.push_back(0);
        _storedWidgetUserData.push_back({0});

        return _produceWidgetId(idx, 0);
    }

    Colibri::Widget* GuiNamespace::_unstoreWidget(WidgetId id){
        uint32_t index;
        WidgetVersion version;
        _readWidgetId(id, &index, &version);
        assert(_storedVersions[index] == version);

        Colibri::Widget* w = _storedPointers[index];
        _storedPointers[index] = 0;
        _storedVersions[index]++; //Increase it here so anything that stores a version becomes invalid.
        memset(&(_storedWidgetUserData[index]), 0, sizeof(GuiWidgetUserData));

        return w;
    }

    Colibri::Widget* GuiNamespace::_getWidget(WidgetId id){
        if(!_isWidgetIdValid(id)) return 0;

        //It might be a bit slow having to do this twice for each id.
        uint32_t index;
        WidgetVersion version;
        _readWidgetId(id, &index, &version);
        return _storedPointers[index];
    }

    GuiNamespace::WidgetId GuiNamespace::_produceWidgetId(uint32_t index, WidgetVersion version){
        return (WidgetId(index) | WidgetId(version) << 32UL);
    }

    void GuiNamespace::_readWidgetId(WidgetId id, uint32_t* outIndex, WidgetVersion* outVersion){
        *outIndex = uint32_t(0) | id;
        *outVersion = uint32_t(0) | id >> 32UL;
    }

    inline bool GuiNamespace::_isWidgetIdValid(WidgetId id){
        uint32_t index;
        WidgetVersion version;
        _readWidgetId(id, &index, &version);

        assert(index < _storedPointers.size());
        assert(index < _storedVersions.size());
        return _storedVersions[index] == version;
    }

    GuiNamespace::WidgetType GuiNamespace::getWidgetTypeFromTypeTag(void* tag){
        if(WidgetButtonTypeTag == tag) return WidgetType::Button;
        else if(WidgetLabelTypeTag == tag) return WidgetType::Label;
        else if(WidgetEditboxTypeTag == tag) return WidgetType::Editbox;
        else if(WidgetSliderTypeTag == tag) return WidgetType::Slider;
        else if(WidgetCheckboxTypeTag == tag) return WidgetType::Checkbox;
        else if(WidgetPanelTypeTag == tag) return WidgetType::Panel;
        else if(WidgetAnimatedLabelTypeTag == tag) return WidgetType::AnimatedLabel;
        else if(WidgetSpinnerTypeTag == tag) return WidgetType::Spinner;
        else return WidgetType::Unknown;
    }

    bool GuiNamespace::getWidgetData(Colibri::Widget* widget, GuiWidgetUserData** outData){
        WidgetId id = widget->m_userId;
        if(!_isWidgetIdValid(id)) return false;

        uint32_t index;
        WidgetVersion version;
        _readWidgetId(id, &index, &version);
        *outData = &(_storedWidgetUserData[index]);

        return true;
    }

    SQInteger GuiNamespace::mapControllerInput(HSQUIRRELVM vm){
        SQInteger button;
        SQInteger inputType;

        sq_getinteger(vm, 2, &button);
        sq_getinteger(vm, 3, &inputType);

        BaseSingleton::getWindow()->getInputMapper()->mapGuiControllerInput(button, static_cast<GuiInputTypes>(inputType));

        return 0;
    }

    SQInteger GuiNamespace::mapKeyboardInput(HSQUIRRELVM vm){
        SQInteger key;
        SQInteger inputType;

        sq_getinteger(vm, 2, &key);
        sq_getinteger(vm, 3, &inputType);

        BaseSingleton::getWindow()->getInputMapper()->mapGuiKeyboardInput(key, static_cast<GuiInputTypes>(inputType));

        return 0;
    }

    SQInteger GuiNamespace::getMousePosGui(HSQUIRRELVM vm){
        const Ogre::Vector2& pos = BaseSingleton::getGuiManager()->getGuiMousePos();

        Vector2UserData::vector2ToUserData(vm, pos);

        return 1;
    }

    SQInteger GuiNamespace::mapControllerAxis(HSQUIRRELVM vm){
        SQInteger axis;
        SQInteger inTop, inBottom, inLeft, inRight;

        sq_getinteger(vm, 2, &axis);
        sq_getinteger(vm, 3, &inTop);
        sq_getinteger(vm, 4, &inBottom);
        sq_getinteger(vm, 5, &inLeft);
        sq_getinteger(vm, 6, &inRight);

        BaseSingleton::getWindow()->getInputMapper()->mapGuiControllerAxis(axis,
            static_cast<GuiInputTypes>(inTop),
            static_cast<GuiInputTypes>(inBottom),
            static_cast<GuiInputTypes>(inLeft),
            static_cast<GuiInputTypes>(inRight)
        );

        return 0;
    }

}
