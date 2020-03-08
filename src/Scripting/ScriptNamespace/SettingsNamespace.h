#pragma once

#include "ScriptUtils.h"

namespace AV{
    class SettingsNamespace{
    public:
        SettingsNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

        static void setupConstants(HSQUIRRELVM vm);

    private:
        static SQInteger getDataDirectory(HSQUIRRELVM vm);
        static SQInteger getMasterDirectory(HSQUIRRELVM vm);
        static SQInteger getWorldSlotSize(HSQUIRRELVM vm);
        static SQInteger getCurrentRenderSystem(HSQUIRRELVM vm);

        static SQInteger getSaveDirectoryViable(HSQUIRRELVM vm);
        static SQInteger getUserSetting(HSQUIRRELVM vm);
    };
}
