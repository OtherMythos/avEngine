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
        //--

        static SQInteger setValue(HSQUIRRELVM vm, bool r);
        static SQInteger getValue(HSQUIRRELVM vm, bool r);

        static SQInteger getInt(HSQUIRRELVM vm, bool r);
        static SQInteger getBool(HSQUIRRELVM vm, bool r);
        static SQInteger getFloat(HSQUIRRELVM vm, bool r);
        static SQInteger getString(HSQUIRRELVM vm, bool r);

        static SQInteger clear(HSQUIRRELVM vm, bool r);
    private:

        inline static bool _isTypeAllowed(SQObjectType t);
        inline static std::shared_ptr<ValueRegistry> _getRegistry(bool registry);
    };
}
