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
        static SQInteger getNumQueries(HSQUIRRELVM vm);
        static SQInteger getMeshByName(HSQUIRRELVM vm);
        static SQInteger createQuery(HSQUIRRELVM vm);
    };
}
