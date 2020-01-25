#pragma once

#include "ScriptUtils.h"

namespace AV{
    class GlobalRegistryNamespace{
    public:
        GlobalRegistryNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger setValue(HSQUIRRELVM vm);

        static SQInteger getInt(HSQUIRRELVM vm);

        inline static bool _isTypeAllowed(SQObjectType t);
    };
}
