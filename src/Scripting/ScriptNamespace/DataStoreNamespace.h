#pragma once

#include "ScriptUtils.h"

namespace AV{
    class ScriptVM;

    class DataStoreNamespace{
        friend ScriptVM;
    public:
        DataStoreNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger getInt(HSQUIRRELVM vm);
        static SQInteger getFloat(HSQUIRRELVM vm);
        static SQInteger getBool(HSQUIRRELVM vm);
        static SQInteger getString(HSQUIRRELVM vm);
        static SQInteger setValue(HSQUIRRELVM vm);
        static SQInteger removeValue(HSQUIRRELVM vm);
    };
}
