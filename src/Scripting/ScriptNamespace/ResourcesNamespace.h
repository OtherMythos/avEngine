#pragma once

#include "ScriptUtils.h"

namespace AV{
    class ResourcesNamespace{
    public:
        ResourcesNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger addResourceLocation(HSQUIRRELVM vm);
        static SQInteger initialiseAllResourceGroups(HSQUIRRELVM vm);
        static SQInteger initialiseResourceGroup(HSQUIRRELVM vm);
        static SQInteger prepareResourceGroup(HSQUIRRELVM vm);
        static SQInteger removeResourceLocation(HSQUIRRELVM vm);
        static SQInteger destroyResourceGroup(HSQUIRRELVM vm);
        static SQInteger getResourceGroups(HSQUIRRELVM vm);
    };
}
