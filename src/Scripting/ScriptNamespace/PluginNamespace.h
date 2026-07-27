#pragma once

#include "ScriptUtils.h"

namespace AV{
    class PluginNamespace{
    public:
        PluginNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger getPlugins(HSQUIRRELVM vm);
        static SQInteger isLoaded(HSQUIRRELVM vm);
    };
}
