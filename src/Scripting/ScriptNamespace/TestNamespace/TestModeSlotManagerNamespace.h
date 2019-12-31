#ifdef TEST_MODE

#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class TestModeSlotManagerNamespace{
    public:
        TestModeSlotManagerNamespace() = delete;

        static void setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled);

    private:
        static SQInteger getQueueSize(HSQUIRRELVM vm);
        static SQInteger getChunkListSize(HSQUIRRELVM vm);
        static SQInteger getChunkActive(HSQUIRRELVM vm);

        static SQInteger activateChunk(HSQUIRRELVM vm);
        static SQInteger constructChunk(HSQUIRRELVM vm);

        static SQInteger getChunkVectorPosition(HSQUIRRELVM vm);

        static SQInteger getNumChunksOfMap(HSQUIRRELVM vm);

        static SQInteger getInUseTerrains(HSQUIRRELVM vm);
        static SQInteger getAvailableTerrains(HSQUIRRELVM vm);
    };
};

#endif
