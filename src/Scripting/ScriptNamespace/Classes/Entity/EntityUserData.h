#pragma once

#include <squirrel.h>
#include "World/Entity/eId.h"
#include "System/Util/DataPacker.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class EntityUserData{
    public:
        EntityUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void eIDToUserData(HSQUIRRELVM vm, eId entity);
        static UserDataGetResult readeIDFromUserData(HSQUIRRELVM vm, int stackIndex, eId* outEID);

    private:
        static SQInteger setEntityPosition(HSQUIRRELVM vm);
        static SQInteger getEntityPosition(HSQUIRRELVM vm);
        static SQInteger moveEntity(HSQUIRRELVM vm);
        static SQInteger moveTowards(HSQUIRRELVM vm);
        static SQInteger checkValid(HSQUIRRELVM vm);
        static SQInteger checkTrackable(HSQUIRRELVM vm);
        static SQInteger isTracked(HSQUIRRELVM vm);
        static SQInteger _entityCompare(HSQUIRRELVM vm);
        static SQInteger getEntityId(HSQUIRRELVM vm);

        static UserDataGetResult _readeIDFromUserDataPointer(HSQUIRRELVM vm, int stackIndex, eId** outEID);

        static SQObject eIdDelegateTable;
    };
}
