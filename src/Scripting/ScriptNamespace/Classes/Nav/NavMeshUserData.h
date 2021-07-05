#pragma once

#include <squirrel.h>
#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "World/Nav/NavTypes.h"

namespace AV{
    class NavMeshUserData{
    public:
        NavMeshUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void meshToUserData(HSQUIRRELVM vm, NavMeshId mesh);

        static UserDataGetResult readMeshFromUserData(HSQUIRRELVM vm, SQInteger stackIndex, NavMeshId* outMeshId);

    private:
        static SQInteger testRay(HSQUIRRELVM vm);
        static SQInteger isMeshValid(HSQUIRRELVM vm);

        static SQObject meshDelegateTable;
    };
}
