#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "World/Slot/SlotManager.h"
#include "World/Slot/Chunk/Chunk.h"

TEST(SlotManagerTests, SetCurrentMapSetsMapVariable){
    AV::SlotManager slot;
    const std::string name = "A Map";

    slot.setCurrentMap(name);

    ASSERT_EQ(name, slot._currentMap);
}

TEST(SlotManagerTests, loadChunkReturnsFalseAfterInvalidCoordinates){
    AV::SlotManager slot;

    bool val = slot.loadChunk("", -1, 0);
    ASSERT_EQ(val, false);

    val = slot.loadChunk("", 0, -1);
    ASSERT_EQ(val, false);
}

TEST(SlotManagerTests, loadChunkReturnsFalseIfChunkIsLoaded){
    AV::SlotManager slot;
    
    //slot._parentSlotNode =

    bool val = slot.loadChunk("Map", 0, 0);
    ASSERT_EQ(val, true);

    val = slot.loadChunk("Map", 0, 0);
    ASSERT_EQ(val, false);
}

TEST(SlotManagerTests, chunkWithinOriginCurrentMapGoesToActiveChunks){
    AV::SlotManager slot;
    const std::string mapName = "map";

    slot.setCurrentMap(mapName);

    slot.loadChunk(mapName, 0, 0);

    ASSERT_EQ(slot._activeChunks.size(), 1);
}

TEST(SlotManagerTests, chunkWithinOriginNotCurrentMapGoesToLoadedChunks){
    AV::SlotManager slot;

    slot.setCurrentMap("map");

    slot.loadChunk("other map", 0, 0);

    ASSERT_EQ(slot._loadedChunks.size(), 1);
}

TEST(SlotManagerTests, chunkOutsideOriginNotCurrentMapGoesToLoadedChunks){
    AV::SlotManager slot;

    slot.setCurrentMap("map");

    slot.loadChunk("other map", 100, 100);

    ASSERT_EQ(slot._loadedChunks.size(), 1);
}

TEST(SlotManagerTests, chunkOutsideOriginCurrentMapGoesToLoadedChunksCurrentMap){
    AV::SlotManager slot;
    const std::string mapName = "map";

    slot.setCurrentMap(mapName);

    slot.loadChunk(mapName, 100, 100);

    ASSERT_EQ(slot._loadedChunksCurrentMap.size(), 1);
}

TEST(SlotManagerTests, checkIfChunkLoadedReturnsTrueWhenChunkLoaded){
    AV::SlotManager slot;
    AV::Chunk* chunk = new AV::Chunk("map", 0, 0, 0);

    slot._activeChunks.push_back(chunk);

    bool val = slot._checkIfChunkLoaded("map", 0, 0);
    ASSERT_TRUE(val);

    slot._activeChunks.pop_back();
    slot._loadedChunks.push_back(chunk);

    val = slot._checkIfChunkLoaded("map", 0, 0);
    ASSERT_TRUE(val);
    slot._loadedChunks.pop_back();

    delete chunk;
}

TEST(SlotManagerTests, checkIfChunkLoadedReturnsFalseIfNotLoaded){
    AV::SlotManager slot;

    bool val = slot._checkIfChunkLoaded("map", 0, 0);

    ASSERT_FALSE(val);
}
