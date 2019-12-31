#pragma once

#include "ScriptUtils.h"

namespace AV{
    class SerialisationNamespace{
    public:
        SerialisationNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger getAvailableSaves(HSQUIRRELVM vm);
        static SQInteger clearAllSaves(HSQUIRRELVM vm);
    };
}
