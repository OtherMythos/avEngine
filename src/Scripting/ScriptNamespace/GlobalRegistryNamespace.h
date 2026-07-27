#pragma once

#include "ScriptUtils.h"

namespace AV{
    class ValueRegistry;

    class GlobalRegistryNamespace{
    public:
        GlobalRegistryNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

        //--
        static SQInteger setValue(HSQUIRRELVM vm);
        static SQInteger getValue(HSQUIRRELVM vm);

        static SQInteger getInt(HSQUIRRELVM vm);
        static SQInteger getBool(HSQUIRRELVM vm);
        static SQInteger getFloat(HSQUIRRELVM vm);
        static SQInteger getString(HSQUIRRELVM vm);
        static SQInteger clear(HSQUIRRELVM vm);
        static SQInteger getKeys(HSQUIRRELVM vm);
        //--
    private:

        inline static bool _isTypeAllowed(SQObjectType t);
    };
}
