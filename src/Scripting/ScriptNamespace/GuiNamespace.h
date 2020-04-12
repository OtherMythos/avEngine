#pragma once

#include "ScriptUtils.h"

namespace Colibri{
    class Widget;
    class LayoutBase;
}

namespace AV{
    class ValueRegistry;

    class GuiNamespace{
    public:
        typedef uint64_t WidgetId;
        typedef uint32_t WidgetVersion;

        GuiNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

        static UserDataGetResult getWidgetFromUserData(HSQUIRRELVM vm, SQInteger idx, Colibri::Widget** outValue);
        static UserDataGetResult getLayoutFromUserData(HSQUIRRELVM vm, SQInteger idx, Colibri::LayoutBase** outValue);

        enum class WidgetType{
            Button,
            Label
        };
        //Create a widget of a specific type, wrap it in a userdata and push it to the stack.
        static void createWidget(HSQUIRRELVM vm, Colibri::Widget* parentWidget, WidgetType type);

    private:
        static SQInteger createWindow(HSQUIRRELVM vm);
        static SQInteger createLayoutLine(HSQUIRRELVM vm);

        /**
        Store a single widget and return its id.

        */
        static WidgetId _storeWidget(Colibri::Widget* widget);
        static void _unstoreWidget(WidgetId id);
        static Colibri::Widget* _getWidget(WidgetId id);
        static WidgetId _produceWidgetId(uint32_t index, WidgetVersion version);
        static void _readWidgetId(WidgetId id, uint32_t* outIndex, WidgetVersion* outVersion);

        static UserDataGetResult _widgetIdFromUserData(HSQUIRRELVM vm, SQInteger idx, WidgetId* outId);
        static void _widgetIdToUserData(HSQUIRRELVM vm, WidgetId id);
        static inline bool _isWidgetIdValid(WidgetId id);

        static SQInteger widgetReleaseHook(SQUserPointer p, SQInteger size);
    };
}
