#pragma once

#include "ScriptUtils.h"

namespace Colibri{
    class Widget;
}

namespace AV{
    class ValueRegistry;

    class GuiNamespace{
    public:
        typedef uint64_t WidgetId;
        typedef uint32_t WidgetVersion;

        GuiNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger createWindow(HSQUIRRELVM vm);

        /**
        Store a single widget and return its id.

        */
        static WidgetId _storeWidget(Colibri::Widget* widget);
        static void _unstoreWidget(WidgetId id);
        static Colibri::Widget* _getWidget(uint64_t id);
        static WidgetId _produceWidgetId(uint32_t index, WidgetVersion version);
        static void _readWidgetId(WidgetId id, uint32_t* outIndex, WidgetVersion* outVersion);

        static void _widgetIdToUserData(HSQUIRRELVM vm, WidgetId id);
        static inline bool _isWidgetIdValid(WidgetId id);

        static SQInteger widgetReleaseHook(SQUserPointer p, SQInteger size);
    };
}
