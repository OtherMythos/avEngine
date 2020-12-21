#pragma once

#include "ScriptUtils.h"

namespace AV{
    class WorldNamespace{
    public:
        WorldNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger createWorld(HSQUIRRELVM vm);
        static SQInteger destroyWorld(HSQUIRRELVM vm);

        static SQInteger getPlayerLoadRadius(HSQUIRRELVM vm);
        static SQInteger setPlayerLoadRadius(HSQUIRRELVM vm);

        static SQInteger setPlayerPosition(HSQUIRRELVM vm);
        static SQInteger getPlayerPosition(HSQUIRRELVM vm);
        static SQInteger getPlayerPositionVec3(HSQUIRRELVM vm);

        static SQInteger worldReady(HSQUIRRELVM vm);
        static SQInteger worldCreatedFromSave(HSQUIRRELVM vm);
        static SQInteger getWorldCreatorHandle(HSQUIRRELVM vm);

        static SQInteger serialiseWorld(HSQUIRRELVM vm);
    };
}
