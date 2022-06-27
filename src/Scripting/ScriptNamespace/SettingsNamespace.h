#pragma once

#include "ScriptUtils.h"

namespace AV{
    class SettingsNamespace{
    public:
        SettingsNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

        static void setupConstants(HSQUIRRELVM vm);

        enum EngineFeatures : uint32{
            FEATURE_TEST_MODE = 1u << 0,
            FEATURE_DEBUGGING_TOOLS = 1u << 1,
        };
    private:
        static SQInteger getDataDirectory(HSQUIRRELVM vm);
        static SQInteger getMasterDirectory(HSQUIRRELVM vm);
        static SQInteger getWorldSlotSize(HSQUIRRELVM vm);
        static SQInteger getCurrentRenderSystem(HSQUIRRELVM vm);
        static SQInteger getEngineFeatures(HSQUIRRELVM vm);
        static SQInteger getPlatform(HSQUIRRELVM vm);
        static SQInteger getOgreResourcesFile(HSQUIRRELVM vm);

        static SQInteger getSaveDirectoryViable(HSQUIRRELVM vm);
        static SQInteger getUserSetting(HSQUIRRELVM vm);
    };
}
