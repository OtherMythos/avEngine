#pragma once

#include <squirrel.h>
#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "World/Nav/NavTypes.h"

namespace AV{
    class NavMeshQueryUserData{
    public:
        NavMeshQueryUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void queryToUserData(HSQUIRRELVM vm, NavMeshId mesh);

        static UserDataGetResult readQueryFromUserData(HSQUIRRELVM vm, SQInteger stackIndex, NavMeshId* outMeshId);

    private:
        static SQInteger isQueryValid(HSQUIRRELVM vm);
        static SQInteger findNearestPoly(HSQUIRRELVM vm);
        static SQInteger moveAlongSurface(HSQUIRRELVM vm);

        static SQInteger navMeshQueryReleaseHook(SQUserPointer p, SQInteger size);

        static SQObject queryDelegateTable;
    };
}
