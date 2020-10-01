#pragma once

#include "ScriptUtils.h"

namespace AV{
    class ScriptVM;
    class NavMeshNamespace{
        friend ScriptVM;
    public:
        NavMeshNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:

        static SQInteger getNumMeshes(HSQUIRRELVM vm);
    };
}