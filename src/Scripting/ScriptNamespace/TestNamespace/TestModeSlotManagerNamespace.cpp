#include "TestModeSlotManagerNamespace.h"

#include "Logger/Log.h"
#include "World/WorldSingleton.h"
#include "World/Slot/SlotManager.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "World/Slot/Chunk/Chunk.h"

#include "OgreSceneNode.h"

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
            SQInteger index = 0;
            sq_getinteger(vm, -1, &index);
            SQBool result = world->getSlotManager()->mTotalChunks[index].second->mActive;
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

    SQInteger TestModeSlotManagerNamespace::getChunkVectorPosition(HSQUIRRELVM vm){
        World* world = WorldSingleton::getWorld();
        if(world){
            SQInteger index = 0;
            sq_getinteger(vm, -1, &index);
            Ogre::Vector3 vec = world->getSlotManager()->mTotalChunks[index].second->getStaticMeshNode()->getPosition();

            //push as array.
            sq_newarray(vm, 3);
            sq_pushfloat(vm, vec.z);
            sq_pushfloat(vm, vec.y);
            sq_pushfloat(vm, vec.x);
            sq_arrayinsert(vm, -4, 0);
            sq_arrayinsert(vm, -3, 1);
            sq_arrayinsert(vm, -2, 2);

            return 1;
        }
        return 0;
    }

    SQInteger TestModeSlotManagerNamespace::getNumChunksOfMap(HSQUIRRELVM vm){
        const SQChar *mapName;
        sq_getstring(vm, -1, &mapName);

        World* world = WorldSingleton::getWorld();
        if(world){
            SQInteger count = world->getSlotManager()->countTotalChunksForMap(Ogre::String(mapName));
            sq_pushinteger(vm, count);

            return 1;
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
        functionMap["getChunkVectorPosition"] = {".i", 2, getChunkVectorPosition};
        functionMap["getNumChunksOfMap"] = {".s", 2, getNumChunksOfMap};

        _redirectFunctionMap(vm, messageFunction, functionMap, testModeEnabled);
    }
}
