#pragma once

#include "ScriptUtils.h"

#include "ColibriGui/ColibriGuiPrerequisites.h"

namespace Colibri{
    class LayoutBase;
}

namespace AV{
    class ValueRegistry;

    class GuiNamespace{
    public:
        typedef uint64_t WidgetId;
        typedef uint32_t WidgetVersion;
        typedef uint8_t ActionType;

        static const ActionType ACTION_ANY = 0xFF;

        GuiNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

        static UserDataGetResult getWidgetFromUserData(HSQUIRRELVM vm, SQInteger idx, Colibri::Widget** outValue, void** outTypeTag);
        static UserDataGetResult getLayoutFromUserData(HSQUIRRELVM vm, SQInteger idx, Colibri::LayoutBase** outValue);

        struct GuiWidgetUserData{
            int userIdx;
        };

        enum class WidgetType{
            Unknown,
            Button,
            Label,
            AnimatedLabel,
            Editbox,
            Slider,
            Checkbox,
            Panel,
            Spinner,
        };
        //Create a widget of a specific type, wrap it in a userdata and push it to the stack.
        static void createWidget(HSQUIRRELVM vm, Colibri::Widget* parentWidget, WidgetType type);
        //Create a window with a specified parent.
        static SQInteger _createWindow(HSQUIRRELVM vm, Colibri::Window* parentWindow, const std::string& winName);

        static SQInteger loadSkins(HSQUIRRELVM vm);

        static SQInteger reprocessMousePosition(HSQUIRRELVM vm);

        /**
        Obtain the user data for a specific widget as a pointer.
        This data is taken from a vector, so no widgets should be created and destroyed while the pointer is still needed.
        */
        static bool getWidgetData(Colibri::Widget* widget, GuiWidgetUserData** outData);

        //Returns true of the passed type tag is any sort of widget other than the window.
        static bool isTypeTagBasicWidget(void* tag);
        static bool isTypeTagWidget(void* tag);

        /**
        Destroy all the widgets created by scripts from this namespace.
        This is intended to be called at engine shutdown.
        */
        static void destroyStoredWidgets();

        //Notify the manager that a widget was destroyed.
        //This is intended to be called by a widget listener.
        static void _notifyWidgetDestruction(Colibri::Widget* widget);
        static void _notifyWidgetActionPerformed(Colibri::Widget* widget, Colibri::Action::Action action);

        static void registerWidgetListener(Colibri::Widget* widget, SQObject targetFunction, SQObject targetContext, WidgetType type, ActionType action);
        //Unbind a listener from a widget. If this widget has no listener it will do nothing.
        static void unbindWidgetListener(Colibri::Widget* widget);

        //Get the number of windows that exist at the moment.
        static int getNumWindows();
        static int getNumWidgets();

        static const std::string* getQueryIdForWindow(Colibri::Window* window);

        static WidgetType getWidgetTypeFromTypeTag(void* tag);

    private:
        static SQInteger createWindow(HSQUIRRELVM vm);
        static SQInteger createLayoutLine(HSQUIRRELVM vm);
        static SQInteger createLayoutTable(HSQUIRRELVM vm);
        static SQInteger destroyWidget(HSQUIRRELVM vm);

        static SQInteger mapControllerInput(HSQUIRRELVM vm);
        static SQInteger mapControllerAxis(HSQUIRRELVM vm);
        static SQInteger mapKeyboardInput(HSQUIRRELVM vm);

        static SQInteger setCanvasSize(HSQUIRRELVM vm);
        static SQInteger setScrollSpeed(HSQUIRRELVM vm);
        static SQInteger setDefaultFontSize26d6(HSQUIRRELVM vm);
        static SQInteger getDefaultFontSize26d6(HSQUIRRELVM vm);
        static SQInteger getOriginalDefaultFontSize26d6(HSQUIRRELVM vm);

        static SQInteger getMousePosGui(HSQUIRRELVM vm);
        static SQInteger simulateMouseButton(HSQUIRRELVM vm);

        static SQInteger getNumWindows(HSQUIRRELVM vm);
        static SQInteger getNumWidgets(HSQUIRRELVM vm);
        static SQInteger getWindowForIdx(HSQUIRRELVM vm);


        static SQInteger colibriWindowToUserData(HSQUIRRELVM vm, Colibri::Window* win);

        /**
        Store a single widget and return its id.

        */
        static WidgetId _storeWidget(Colibri::Widget* widget);
        /**
        Remove a widget from the store system.

        @returns
        The pointer to the removed widget.
        */
        static Colibri::Widget* _unstoreWidget(WidgetId id);
        static Colibri::Widget* _getWidget(WidgetId id);
        static WidgetId _produceWidgetId(uint32_t index, WidgetVersion version);
        static void _readWidgetId(WidgetId id, uint32_t* outIndex, WidgetVersion* outVersion);

        static UserDataGetResult _widgetIdFromUserData(HSQUIRRELVM vm, SQInteger idx, WidgetId* outId, void** outTypeTag);
        static void _widgetIdToUserData(HSQUIRRELVM vm, WidgetId id, void* typeTag);
        static inline bool _isWidgetIdValid(WidgetId id);

        static SQInteger widgetReleaseHook(SQUserPointer p, SQInteger size);
        static SQInteger layoutReleaseHook(SQUserPointer p, SQInteger size);
    };
}
