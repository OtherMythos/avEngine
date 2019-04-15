#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "World/Slot/SlotManager.h"
#include "World/Slot/ChunkRadiusLoader.h"
#include "World/Slot/Chunk/ChunkFactory.h"
#include "World/WorldSingleton.h"
#include "System/SystemSetup/SystemSettings.h"

class SlotManagerMock : public AV::SlotManager{
public:
    SlotManagerMock(std::shared_ptr<AV::ChunkFactory> chunkFactory) : SlotManager(chunkFactory) {};
    
    MOCK_METHOD1(activateChunk, bool(const AV::ChunkCoordinate &coord));
    MOCK_METHOD1(destroyChunk, bool(const AV::ChunkCoordinate &coord));
};

TEST(ChunkRadiusChecksTests, rectCircleCollisionTests){
    std::unique_ptr<AV::ChunkRadiusLoader> chunk;
    
    //Sensible val close to the origin
    bool val = chunk->_checkRectCircleCollision(0, 0, 100, 10, 0, 0);
    ASSERT_TRUE(val);

    //Checking a tile of tile at 10 against the origin.
    val = chunk->_checkRectCircleCollision(10, 10, 100, 10, 0, 0);
    ASSERT_FALSE(val);
    
    //A tile that would be caught in the rectangle but not in the circle
    val = chunk->_checkRectCircleCollision(10, 10, 10, 100, 0, 0);
    ASSERT_FALSE(val);
}

TEST(ChunkRadiusChecksTests, unloadEverythingClearsList){
    //TODO maybe clean this up.
    auto ptr = new SlotManagerMock(std::make_shared<AV::ChunkFactory>());
    std::shared_ptr<AV::SlotManager> slot(ptr);
    std::unique_ptr<AV::ChunkRadiusLoader> chunk(new AV::ChunkRadiusLoader(slot));
    
    EXPECT_CALL(*ptr, destroyChunk(::testing::_)).Times(2);
    
    chunk->mLoadedChunks.insert(AV::ChunkRadiusLoader::LoadedChunkData(1, 1));
    chunk->mLoadedChunks.insert(AV::ChunkRadiusLoader::LoadedChunkData(2, 2));
    chunk->_unloadEverything();
    
    ASSERT_EQ(chunk->mLoadedChunks.size(), 0);
}

TEST(ChunkRadiusChecksTests, updatePlayerUnloadsEverythingIf0Radius){
    auto ptr = new SlotManagerMock(std::make_shared<AV::ChunkFactory>());
    std::shared_ptr<AV::SlotManager> slot(ptr);
    std::unique_ptr<AV::ChunkRadiusLoader> chunk(new AV::ChunkRadiusLoader(slot));
    
    EXPECT_CALL(*ptr, destroyChunk(::testing::_)).Times(1);
    
    AV::WorldSingleton::mPlayerLoadRadius = 0;
    chunk->mLoadedChunks.insert(AV::ChunkRadiusLoader::LoadedChunkData(1, 1));
    
    chunk->_updatePlayer(AV::SlotPosition());
    
    ASSERT_EQ(chunk->mLoadedChunks.size(), 0);
}

TEST(ChunkRadiusChecksTests, updatePlayerUnloadsStaleChunks){
    auto ptr = new SlotManagerMock(std::make_shared<AV::ChunkFactory>());
    std::shared_ptr<AV::SlotManager> slot(ptr);
    std::unique_ptr<AV::ChunkRadiusLoader> chunk(new AV::ChunkRadiusLoader(slot));
    
    AV::SystemSettings::_worldSlotSize = 100;
    AV::WorldSingleton::mPlayerLoadRadius = 100;
    
    AV::ChunkCoordinate coord(200, 200, AV::WorldSingleton::getCurrentMap());
    //We expect some calls to activate chunk, even though this is testing unloads.
    EXPECT_CALL(*ptr, activateChunk(::testing::_)).Times(::testing::AtLeast(1));
    //The one loaded chunk should be checked with those coordinates.
    EXPECT_CALL(*ptr, destroyChunk(coord)).Times(1);
    
    //Insert some obviously stale chunks.
    chunk->mLoadedChunks.insert(AV::ChunkRadiusLoader::LoadedChunkData(coord.chunkX(), coord.chunkY()));
    
    chunk->_updatePlayer(AV::SlotPosition());
    
    //We can't assert the size of the list because this call will also load some stuff.
    //We're only interested in the unloads for now though, so expecting destroy chunk is fine.
}

TEST(ChunkRadiusChecksTests, updatePlayerLoadsChunks){
    auto ptr = new SlotManagerMock(std::make_shared<AV::ChunkFactory>());
    std::shared_ptr<AV::SlotManager> slot(ptr);
    std::unique_ptr<AV::ChunkRadiusLoader> chunk(new AV::ChunkRadiusLoader(slot));

    AV::SystemSettings::_worldSlotSize = 100;
    AV::WorldSingleton::mPlayerLoadRadius = 50;
    //If it's at the origin, it should be 4.
    EXPECT_CALL(*ptr, activateChunk(::testing::_)).Times(4);
    
    chunk->_updatePlayer(AV::SlotPosition());
    
    //Check that there are that many entries in the loaded list.
    ASSERT_EQ(chunk->mLoadedChunks.size(), 4);
}
