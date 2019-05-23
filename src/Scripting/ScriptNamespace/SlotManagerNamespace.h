#pragma once

#include "ScriptNamespace.h"
#include "World/Slot/ChunkCoordinate.h"

namespace AV{
    class SlotManagerNamespace : public ScriptNamespace{
    public:
        SlotManagerNamespace() {};

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger setOrigin(HSQUIRRELVM vm);
        static SQInteger setCurrentMap(HSQUIRRELVM vm);
        static SQInteger loadChunk(HSQUIRRELVM vm);
        static SQInteger unloadChunk(HSQUIRRELVM vm);
        static SQInteger getCurrentMap(HSQUIRRELVM vm);

        static ChunkCoordinate _getChunkCoordPopStack(HSQUIRRELVM vm);
    };
};
