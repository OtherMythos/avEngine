#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class TerrainObject;

    class TerrainObjectUserData{
    public:
        TerrainObjectUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void TerrainObjectToUserData(HSQUIRRELVM vm, TerrainObject* object);

        static UserDataGetResult readTerrainObjectFromUserData(HSQUIRRELVM vm, SQInteger stackInx, TerrainObject** outObject);

    private:

        static SQObject terrainObjectDelegateTableObject;

        static SQInteger setRenderQueueGroup(HSQUIRRELVM vm);
        static SQInteger update(HSQUIRRELVM vm);
        static SQInteger load(HSQUIRRELVM vm);

    };
}
