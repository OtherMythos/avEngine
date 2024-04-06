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
#include "ColibriGui/Layouts/ColibriLayoutTableSameSize.h"
#include "Gui/AnimatedLabel.h"

#include "Window/Window.h"
#include "Window/InputMapper.h"
#include "Window/GuiInputProcessor.h"

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

    struct WindowEntry{
        Colibri::Window* win;
        std::string name;
    };
    //A list of all the windows ever created, unless they've been properly destroyed. This is used for correct destruction of objects during engine shutdown.
    static std::vector<WindowEntry> _createdWindows;
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
    static SQObject sizerLayoutTableDelegateTable;

    static GuiNamespaceWidgetListener mNamespaceWidgetListener;
    static GuiNamespaceWidgetActionListener mNamespaceWidgetActionListener;

    //typedef std::pair<SQObject, GuiNamespace::WidgetType> ListenerFunction;
    struct ListenerFunction{
        SQObject first;
        GuiNamespace::WidgetType second;
        SQObject context;
        GuiNamespace::ActionType actionType;
    };
    static std::map<GuiNamespace::WidgetId, ListenerFunction> _attachedListeners;
    static HSQUIRRELVM _vm; //A static reference to the vm for the action callback functions.

    static const uint32_t _listenerMask = Colibri::ActionMask::Cancel | Colibri::ActionMask::Highlighted | Colibri::ActionMask::Hold | Colibri::ActionMask::PrimaryActionPerform | Colibri::ActionMask::SecondaryActionPerform | Colibri::ActionMask::ValueChanged;

    int GuiNamespace::getNumWindows(){
        return static_cast<int>(_createdWindows.size());
    }

    int GuiNamespace::getNumWidgets(){
        int count = 0;

        for(Colibri::Widget* w : _storedPointers){
            if(!w) continue;

            if(!w->isWindow()) count++;
        }

        return count;
    }

    SQInteger GuiNamespace::getWindowForIdx(HSQUIRRELVM vm){
        SQInteger winIdx;
        sq_getinteger(vm, 2, &winIdx);

        if(winIdx < 0 || winIdx >= _createdWindows.size()) return sq_throwerror(vm, "Invalid window index.");

        //colibriWindowToUserData(vm, _createdWindows[winIdx].win);
        widgetToUserData(vm, _createdWindows[winIdx].win);
        return 1;
    }

    const std::string* GuiNamespace::getQueryIdForWindow(Colibri::Window* window){
        const std::string* outString = 0;
        for(const WindowEntry& e : _createdWindows){
            if(e.win == window){
                outString = &e.name;
            }
        }

        return outString;
    }

    SQInteger GuiNamespace::colibriWindowToUserData(HSQUIRRELVM vm, Colibri::Window* win){
        /*
        WidgetId id = _storeWidget(win, WidgetType::Window);
        win->m_userId = id;
        win->addListener(&mNamespaceWidgetListener);
        _widgetIdToUserData(vm, id, WidgetWindowTypeTag);
        sq_pushobject(vm, windowDelegateTable);
        sq_setdelegate(vm, -2);
        */

        //_storeWidget(win, WidgetType::Window);
        widgetToUserData(vm, win);

        return 1;
    }

    SQInteger GuiNamespace::_createWindow(HSQUIRRELVM vm, Colibri::Window* parentWindow, const std::string& winName){
        Colibri::Window* win = BaseSingleton::getGuiManager()->getColibriManager()->createWindow(parentWindow);
        //By default disable scroll.
        win->setMaxScroll(Ogre::Vector2::ZERO);
        win->addListener(&mNamespaceWidgetListener);
        _createdWindows.push_back({win, winName});

        WidgetId outId = _storeWidget(win, WidgetType::Window);
        win->m_userId = outId;
        widgetToUserData(vm, win);

        return 1;

        //return colibriWindowToUserData(vm, win);
    }

    SQInteger GuiNamespace::createWindow(HSQUIRRELVM vm){
        std::string title = "";
        Colibri::Window* parent = 0;
        SQInteger topIdx = sq_gettop(vm);
        if(topIdx >= 2){
            const char* titleChar;
            sq_getstring(vm, 2, &titleChar);
            title = titleChar;
        }
        if(topIdx >= 3){
            void* expectedType;
            Colibri::Widget* outWidget = 0;
            SCRIPT_CHECK_RESULT(getWidgetFromUserData(vm, 3, &outWidget, &expectedType));
            if(expectedType != WidgetWindowTypeTag) return sq_throwerror(vm, "Only a window as a parent can be provided.");
            parent = dynamic_cast<Colibri::Window*>(outWidget);
            assert(parent);
        }

        return _createWindow(vm, parent, title);
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

    SQInteger GuiNamespace::createLayoutTable(HSQUIRRELVM vm){
        //They're just created on the heap for now. There's no tracking of these pointers in this class.
        //When the object goes out of scope the release hook destroys it.
        Colibri::LayoutTableSameSize* table = new Colibri::LayoutTableSameSize(BaseSingleton::getGuiManager()->getColibriManager());

        Colibri::LayoutBase** pointer = (Colibri::LayoutBase**)sq_newuserdata(vm, sizeof(Colibri::LayoutBase*));
        *pointer = table;

        sq_setreleasehook(vm, -1, layoutReleaseHook);
        sq_settypetag(vm, -1, LayoutTableTypeTag);

        sq_pushobject(vm, sizerLayoutTableDelegateTable);

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

    SQInteger GuiNamespace::setDefaultFontSize26d6(HSQUIRRELVM vm){
        SQInteger defaultFontSize;
        sq_getinteger(vm, -1, &defaultFontSize);

        BaseSingleton::getGuiManager()->getColibriManager()->setDefaultFontSize26d6(static_cast<uint32_t>(defaultFontSize));

        return 0;
    }

    SQInteger GuiNamespace::getDefaultFontSize26d6(HSQUIRRELVM vm){
        uint32_t defaultFontSize = BaseSingleton::getGuiManager()->getColibriManager()->getDefaultFontSize26d6();

        sq_pushinteger(vm, defaultFontSize);
        return 1;
    }

    SQInteger GuiNamespace::getOriginalDefaultFontSize26d6(HSQUIRRELVM vm){
        uint32_t defaultFontSize = BaseSingleton::getGuiManager()->getOriginalDefaultFontSize26d6();

        sq_pushinteger(vm, defaultFontSize);
        return 1;
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

    SQInteger GuiNamespace::reprocessMousePosition(HSQUIRRELVM vm){
        BaseSingleton::getGuiManager()->reprocessMousePosition();

        return 0;
    }

    SQInteger GuiNamespace::simulateMouseButton(HSQUIRRELVM vm){
        SQInteger mouseButton;
        SQBool mousePressed;

        sq_getinteger(vm, 2, &mouseButton);
        sq_getbool(vm, 3, &mousePressed);

        bool guiConsumed = BaseSingleton::getGuiInputProcessor()->processMouseButton(static_cast<int>(mouseButton), mousePressed);
        sq_pushbool(vm, guiConsumed);

        return 1;
    }

    SQInteger GuiNamespace::getNumWindows(HSQUIRRELVM vm){
        sq_pushinteger(vm, getNumWindows());

        return 1;
    }

    SQInteger GuiNamespace::getNumWidgets(HSQUIRRELVM vm){
        sq_pushinteger(vm, getNumWidgets());

        return 1;
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
        ScriptUtils::setupDelegateTable(vm, &sizerLayoutTableDelegateTable, GuiSizerDelegate::setupLayoutTable);

        /**SQNamespace
        @name _gui
        @desc A namespace to create and control gui elements.
        */

        /**SQFunction
        @name createWindow
        @desc Create a window.
        @param1:String:Window query id.
        @param2:Window:Parent window.
        @returns A window object.
        */
        ScriptUtils::addFunction(vm, createWindow, "createWindow", -1, ".su");
        /**SQFunction
        @name createLayoutLine
        @desc Create a layout line object.
        @param1:alignment:An iterator specifying whether this is a horizontal or vertical layout line.
        @returns A layout line object.
        */
        ScriptUtils::addFunction(vm, createLayoutLine, "createLayoutLine", -1, ".i");
        /**SQFunction
        @name createLayoutTable
        @desc Create a layout table object.
        @returns A layout table object.
        */
        ScriptUtils::addFunction(vm, createLayoutTable, "createLayoutTable");
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
        @name setDefaultFontSize26d6
        @desc Set the global default font size using the measuring system of 26d6. All created widgets will base themselves upon this.
        @param1:Integer:The new default font size.
        */
        ScriptUtils::addFunction(vm, setDefaultFontSize26d6, "setDefaultFontSize26d6", 2, ".i");
        /**SQFunction
        @name getDefaultFontSize26d6
        @desc Get the global default font size using the measuring system of 26d6.
        */
        ScriptUtils::addFunction(vm, getDefaultFontSize26d6, "getDefaultFontSize26d6");
        /**SQFunction
        @name getOriginalDefaultFontSize26d6
        @desc Get the original default font size. This won't be effected by a change to the default size.
        */
        ScriptUtils::addFunction(vm, getOriginalDefaultFontSize26d6, "getOriginalDefaultFontSize26d6");


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
        @name reprocessMousePosition
        @desc Force a re-process of the mouse position, without requiring a mouse move. This is useful if say a button press recently caused a complete change to a user interface.
        */
        ScriptUtils::addFunction(vm, reprocessMousePosition, "reprocessMousePosition");

        /**SQFunction
        @name mapControllerInput
        @desc Map a controller input to a gui input.
        */
        ScriptUtils::addFunction(vm, mapControllerInput, "mapControllerInput", 3, ".ii");
        /**SQFunction
        @name mapKeyboardInput
        @desc Map a keyboard input to a gui input.
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
        /**SQFunction
        @name simulateMouseButton
        @desc Simulate a mouse button press or release. Useful for automation and unique use cases.
        @return Whether the interaction was consumed by the gui.
        */
        ScriptUtils::addFunction(vm, simulateMouseButton, "simulateMouseButton", 3, ".ib");
        /**SQFunction
        @name getWindowForIdx
        @desc Return an active window for a specific index, allowing iteration of the windows.
        @return GuiWindow or error if the index is invalid.
        */
        ScriptUtils::addFunction(vm, getWindowForIdx, "getWindowForIdx", 2, ".i");
        /**SQFunction
        @name getNumWindows
        @return The number of active windows.
        */
        ScriptUtils::addFunction(vm, getNumWindows, "getNumWindows");
        /**SQFunction
        @name getNumWidgets
        @return The number of active widgets.
        */
        ScriptUtils::addFunction(vm, getNumWidgets, "getNumWidgets");

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
        @name _GUI_ACTION_CANCEL
        @desc GUI widget action canceled.
        */
        ScriptUtils::declareConstant(vm, "_GUI_ACTION_CANCEL", Colibri::Action::Cancel);
        /**SQConstant
        @name _GUI_ACTION_HIGHLIGHTED
        @desc GUI widget action when focus is gained.
        */
        ScriptUtils::declareConstant(vm, "_GUI_ACTION_HIGHLIGHTED", Colibri::Action::Highlighted);
        /**SQConstant
        @name _GUI_ACTION_PRESSED
        @desc GUI widget action when the widget is pressed. This includes press from keyboard.
        */
        ScriptUtils::declareConstant(vm, "_GUI_ACTION_PRESSED", Colibri::Action::Hold);
        /**SQConstant
        @name _GUI_ACTION_RELEASED
        @desc GUI wiget action when the widget press ends.
        */
        ScriptUtils::declareConstant(vm, "_GUI_ACTION_RELEASED", Colibri::Action::PrimaryActionPerform);
        //I don't think this constant is needed as colibri doesn't seem to use it.
        // /**SQConstant
        // @name _GUI_ACTION_RELEASED_SECONDARY
        // @desc GUI wiget action when the widget press ends from the secondary input.
        // */
        // ScriptUtils::declareConstant(vm, "_GUI_ACTION_RELEASED_SECONDARY", Colibri::Action::SecondaryActionPerform);
        /**SQConstant
        @name _GUI_ACTION_VALUE_CHANGED
        @desc GUI wiget action for when the value changes, for instance in a spinner widget.
        */
        ScriptUtils::declareConstant(vm, "_GUI_ACTION_VALUE_CHANGED", Colibri::Action::ValueChanged);


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

        /**SQConstant
        @name _GUI_WIDGET_BUTTON
        */
        ScriptUtils::declareConstant(vm, "_GUI_WIDGET_BUTTON", static_cast<SQInteger>(WidgetType::Button));
        /**SQConstant
        @name _GUI_WIDGET_LABEL
        */
        ScriptUtils::declareConstant(vm, "_GUI_WIDGET_LABEL", static_cast<SQInteger>(WidgetType::Label));
        /**SQConstant
        @name _GUI_WIDGET_ANIMATED_LABEL
        */
        ScriptUtils::declareConstant(vm, "_GUI_WIDGET_ANIMATED_LABEL", static_cast<SQInteger>(WidgetType::AnimatedLabel));
        /**SQConstant
        @name _GUI_WIDGET_EDITBOX
        */
        ScriptUtils::declareConstant(vm, "_GUI_WIDGET_EXITBOX", static_cast<SQInteger>(WidgetType::Editbox));
        /**SQConstant
        @name _GUI_WIDGET_SLIDER
        */
        ScriptUtils::declareConstant(vm, "_GUI_WIDGET_SLIDER", static_cast<SQInteger>(WidgetType::Slider));
        /**SQConstant
        @name _GUI_WIDGET_CHECKBOX
        */
        ScriptUtils::declareConstant(vm, "_GUI_WIDGET_CHECKBOX", static_cast<SQInteger>(WidgetType::Checkbox));
        /**SQConstant
        @name _GUI_WIDGET_PANEL
        */
        ScriptUtils::declareConstant(vm, "_GUI_WIDGET_PANEL", static_cast<SQInteger>(WidgetType::Panel));
        /**SQConstant
        @name _GUI_WIDGET_SPINNER
        */
        ScriptUtils::declareConstant(vm, "_GUI_WIDGET_SPINNER", static_cast<SQInteger>(WidgetType::Spinner));
        /**SQConstant
        @name _GUI_WIDGET_WINDOW
        */
        ScriptUtils::declareConstant(vm, "_GUI_WIDGET_WINDOW", static_cast<SQInteger>(WidgetType::Window));

    }

    void GuiNamespace::_notifyWidgetDestruction(Colibri::Widget* widget){
        WidgetId id = widget->m_userId;
        _unstoreWidget(id);


        unbindWidgetListener(widget);

        if(widget->isWindow()){
            auto it = std::find_if(_createdWindows.begin(), _createdWindows.end(), [widget] (const WindowEntry& e) {
                return e.win == widget;
            });
            assert(it != _createdWindows.end());
            //if(it != _createdWindows.end()){
                _createdWindows.erase(it);
            //}
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

    void GuiNamespace::registerWidgetListener(Colibri::Widget* widget, SQObject targetFunction, SQObject targetContext, WidgetType type, ActionType action){
        WidgetId id = widget->m_userId;
        if(!_isWidgetIdValid(id)) return;

        //It's now confirmed for storage, so increase the reference so it's not destroyed until its released.
        sq_addref(_vm, &targetFunction);
        if(targetContext._type != OT_NULL) sq_addref(_vm, &targetContext);
        _attachedListeners[id] = {targetFunction, type, targetContext, action};

        widget->addActionListener(&mNamespaceWidgetActionListener, _listenerMask);
    }

    void GuiNamespace::_notifyWidgetActionPerformed(Colibri::Widget* widget, Colibri::Action::Action action){
        WidgetId id = widget->m_userId;
        auto it = _attachedListeners.find(id);
        if(it == _attachedListeners.end()) return;

        GuiNamespace::ActionType actionType = static_cast<GuiNamespace::ActionType>((*it).second.actionType);
        if(action != static_cast<Colibri::Action::Action>(actionType) && actionType != GuiNamespace::ACTION_ANY) return;

        sq_pushobject(_vm, (*it).second.first);

        SQObject targetContext = (*it).second.context;
        if(targetContext._type == OT_NULL){
            sq_pushroottable(_vm);
        }else{
            sq_pushobject(_vm, targetContext);
        }

        widgetToUserData(_vm, widget);

        sq_pushinteger(_vm, (SQInteger)action);

        SQInteger paramCount = 3;

        if(SQ_FAILED(sq_call(_vm, paramCount, false, true))){
            //return false;
        }
    }

    void GuiNamespace::destroyStoredWidgets(){
        Colibri::ColibriManager* man = BaseSingleton::getGuiManager()->getColibriManager();
        //Destroy in a while loop, as sometimes windows can be children of windows.
        //These items might be removed during the loop.
        while(!_createdWindows.empty()){
            //This function calls delete on the pointer, as well as all its children.
            Colibri::Window* target = _createdWindows.back().win;
            assert(target);
            man->destroyWindow(target);
            //Destroying the window here should automatically remove it from the list.
        }
        assert(_createdWindows.empty());
        _storedPointers.clear();
        _storedVersions.clear();
        _storedWidgetUserData.clear();
    }

    UserDataGetResult GuiNamespace::getLayoutFromUserData(HSQUIRRELVM vm, SQInteger idx, Colibri::LayoutBase** outValue){
        SQUserPointer pointer, typeTag;
        if(!SQ_SUCCEEDED(sq_getuserdata(vm, idx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != LayoutLineTypeTag && typeTag != LayoutTableTypeTag){
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
        switch(type){
            case WidgetType::Button:
                w = man->createWidget<Colibri::Button>(parentWidget);
                break;
            case WidgetType::Label:
                w = man->createWidget<Colibri::Label>(parentWidget);
                break;
            case WidgetType::AnimatedLabel:
                w = man->createWidget<AnimatedLabel>(parentWidget);
                break;
            case WidgetType::Editbox:
                w = man->createWidget<Colibri::Editbox>(parentWidget);
                break;
            case WidgetType::Slider:
                w = man->createWidget<Colibri::Slider>(parentWidget);
                break;
            case WidgetType::Checkbox:
                w = man->createWidget<Colibri::Checkbox>(parentWidget);
                break;
            case WidgetType::Panel:{
                w = man->createWidget<Colibri::WrappedColibriRenderable>(parentWidget);
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
                break;
            default:
                assert(false);
                break;
        }

        WidgetId id = _storeWidget(w, type);
        w->m_userId = id;
        //OPTIMISATION Each widget seems to contain a vector for listeners, however I only need the one.
        //It seems to me like modifications to the library might help fix this.
        w->addListener(&mNamespaceWidgetListener);
        //_widgetIdToUserData(vm, id, typeTag);
        widgetToUserData(vm, w);
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

    void GuiNamespace::_getTypeTagAndDelegate(WidgetType type, void** typeTag, SQObject** delegateTable){
        switch(type){
            case WidgetType::Editbox:
                *typeTag = WidgetEditboxTypeTag;
                *delegateTable = &editboxDelegateTable;
                break;
            case WidgetType::Button:
                *typeTag = WidgetButtonTypeTag;
                *delegateTable = &buttonDelegateTable;
                break;
            case WidgetType::Slider:
                *typeTag = WidgetSliderTypeTag;
                *delegateTable = &sliderDelegateTable;
                break;
            case WidgetType::Checkbox:
                *typeTag = WidgetCheckboxTypeTag;
                *delegateTable = &checkboxDelegateTable;
                break;
            case WidgetType::Panel:
                *typeTag = WidgetPanelTypeTag;
                *delegateTable = &panelDelegateTable;
                break;
            case WidgetType::Spinner:
                *typeTag = WidgetSpinnerTypeTag;
                *delegateTable = &spinnerDelegateTable;
                break;
            case WidgetType::Label:
                *typeTag = WidgetLabelTypeTag;
                *delegateTable = &labelDelegateTable;
                break;
            case WidgetType::Window:
                *typeTag = WidgetWindowTypeTag;
                *delegateTable = &windowDelegateTable;
                break;
            case WidgetType::AnimatedLabel:
                *typeTag = WidgetAnimatedLabelTypeTag;
                *delegateTable = &animatedLabelDelegateTable;
                break;
            default:
                assert(false);
                break;
        }
    }

    void GuiNamespace::widgetToUserData(HSQUIRRELVM vm, Colibri::Widget* widget){
        WidgetId id = widget->m_userId;

        GuiWidgetUserData* userData;
        bool result = getWidgetData(widget, &userData);
        //The widget must have been stored in order to convert to user data.
        assert(result);

        WidgetType widgetType = userData->type;

        WidgetId* pointer = (WidgetId*)sq_newuserdata(vm, sizeof(WidgetId));
        *pointer = id;

        void* typeTag;
        SQObject* delegateTable;
        _getTypeTagAndDelegate(widgetType, &typeTag, &delegateTable);

        sq_pushobject(_vm, *delegateTable);
        sq_setdelegate(_vm, -2);

        sq_setreleasehook(vm, -1, widgetReleaseHook);
        sq_settypetag(vm, -1, typeTag);
    }

    GuiNamespace::WidgetId GuiNamespace::_storeWidget(Colibri::Widget* widget, WidgetType widgetType){
        assert(_storedVersions.size() == _storedPointers.size()); //Each pointer should have a version

        //OPTIMISATION rather than searching the entire list linearly I could have some sort of skip list.
        for(uint32_t i = 0; i < _storedPointers.size(); i++){
            if(_storedPointers[i] != 0x0) continue;

            //An empty entry was found.
            _storedPointers[i] = widget;
            _storedWidgetUserData[i].type = widgetType;
            _storedWidgetUserData[i].userIdx = 0;
            //I don't increase the version here. That's done during the deletion.

            return _produceWidgetId(i, _storedVersions[i]);
        }

        //No space in the list was found, so just create one.
        uint32_t idx = static_cast<uint32_t>(_storedPointers.size());
        _storedPointers.push_back(widget);
        _storedVersions.push_back(0);
        _storedWidgetUserData.push_back({0, widgetType});

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
        *outIndex = uint32_t(0) | static_cast<uint32_t>(id);
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

        BaseSingleton::getWindow()->getInputMapper()->mapGuiControllerInput((int)button, static_cast<GuiInputTypes>(inputType));

        return 0;
    }

    SQInteger GuiNamespace::mapKeyboardInput(HSQUIRRELVM vm){
        SQInteger key;
        SQInteger inputType;

        sq_getinteger(vm, 2, &key);
        sq_getinteger(vm, 3, &inputType);

        BaseSingleton::getWindow()->getInputMapper()->mapGuiKeyboardInput((int)key, static_cast<GuiInputTypes>(inputType));

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

        BaseSingleton::getWindow()->getInputMapper()->mapGuiControllerAxis((int)axis,
            static_cast<GuiInputTypes>(inTop),
            static_cast<GuiInputTypes>(inBottom),
            static_cast<GuiInputTypes>(inLeft),
            static_cast<GuiInputTypes>(inRight)
        );

        return 0;
    }

}
