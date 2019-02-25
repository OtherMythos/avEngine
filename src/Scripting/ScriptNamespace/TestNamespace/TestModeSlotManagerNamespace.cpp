#include "TestModeSlotManagerNamespace.h"

#include "Logger/Log.h"
#include "World/WorldSingleton.h"
#include "World/Slot/SlotManager.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "World/Slot/Chunk/Chunk.h"

namespace AV{
    SQInteger TestModeSlotManagerNamespace::getQueueSize(HSQUIRRELVM vm){
        World* world = WorldSingleton::getWorld();
        if(world){
            SQInteger size = world->getSlotManager()->queuedEntries.size();
            sq_pushinteger(vm, size);
            return 1;
        }
        return 0;
    }

    SQInteger TestModeSlotManagerNamespace::getChunkListSize(HSQUIRRELVM vm){
        World* world = WorldSingleton::getWorld();
        if(world){
            SQInteger size = world->getSlotManager()->mTotalChunks.size();
            sq_pushinteger(vm, size);
            return 1;
        }
        return 0;
    }
    
    SQInteger TestModeSlotManagerNamespace::getChunkActive(HSQUIRRELVM vm){
        World* world = WorldSingleton::getWorld();
        if(world){
            SQBool result = world->getSlotManager()->mTotalChunks[0].second->mActive;
            sq_pushbool(vm, result);
            return 1;
        }
        return 0;
    }

    SQInteger TestModeSlotManagerNamespace::activateChunk(HSQUIRRELVM vm){
        World* world = WorldSingleton::getWorld();
        if(world){
            ChunkCoordinate coord = ScriptUtils::getChunkCoordPopStack(vm);

            world->getSlotManager()->activateChunk(coord);
        }
        return 0;
    }
    
    SQInteger TestModeSlotManagerNamespace::constructChunk(HSQUIRRELVM vm){
        World* world = WorldSingleton::getWorld();
        if(world){
            ChunkCoordinate coord = ScriptUtils::getChunkCoordPopStack(vm);
            
            world->getSlotManager()->constructChunk(coord);
        }
        return 0;
    }

    void TestModeSlotManagerNamespace::setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled){
        RedirectFunctionMap functionMap;
        functionMap["getQueueSize"] = {"", 0, getQueueSize};
        functionMap["getChunkListSize"] = {"", 0, getChunkListSize};
        functionMap["constructChunk"] = {".sii", 4, constructChunk};
        functionMap["activateChunk"] = {".sii", 4, activateChunk};
        functionMap["getChunkActive"] = {".i", 2, getChunkActive};

        _redirectFunctionMap(vm, messageFunction, functionMap, testModeEnabled);
    }
}
