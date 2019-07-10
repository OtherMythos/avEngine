#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "World/Slot/SlotManager.h"
#include "World/Slot/ChunkRadiusLoader.h"
#include "World/Slot/Chunk/ChunkFactory.h"
#include "World/WorldSingleton.h"
#include "System/SystemSetup/SystemSettings.h"

#include "unit/src/World/Physics/PhysicsManagerMock.h"

class SlotManagerMock : public AV::SlotManager{
public:
    SlotManagerMock(std::shared_ptr<AV::ChunkFactory> chunkFactory) : SlotManager(chunkFactory) {};

    MOCK_METHOD1(activateChunk, bool(const AV::ChunkCoordinate &coord));
    MOCK_METHOD1(destroyChunk, bool(const AV::ChunkCoordinate &coord));
};

class ChunkRadiusChecksTests : public ::testing::Test {
private:
    std::shared_ptr<SlotManagerMock> slotManager;

    AV::ChunkRadiusLoader* chunkRadiusLoader;
public:
    ChunkRadiusChecksTests() {
    }

    virtual ~ChunkRadiusChecksTests() {
    }

    virtual void SetUp() {
        //slotManager = new SlotManagerMock(std::make_shared<AV::ChunkFactory>(std::make_shared<PhysicsManagerMock>(), bodyConstructor));
        slotManager = std::make_shared<SlotManagerMock>(std::make_shared<AV::ChunkFactory>(std::make_shared<PhysicsManagerMock>()));
        //tracker = new AV::EntityTracker();
        chunkRadiusLoader = new AV::ChunkRadiusLoader(slotManager);
    }

    virtual void TearDown() {
        //delete tracker;
        delete chunkRadiusLoader;
        //delete slotManager;
    }
};

TEST_F(ChunkRadiusChecksTests, rectCircleCollisionTests){
    //Sensible val close to the origin
    bool val = chunkRadiusLoader->_checkRectCircleCollision(0, 0, 100, 10, 0, 0);
    ASSERT_TRUE(val);

    //Checking a tile of tile at 10 against the origin.
    val = chunkRadiusLoader->_checkRectCircleCollision(10, 10, 100, 10, 0, 0);
    ASSERT_FALSE(val);

    //A tile that would be caught in the rectangle but not in the circle
    val = chunkRadiusLoader->_checkRectCircleCollision(10, 10, 10, 100, 0, 0);
    ASSERT_FALSE(val);
}

TEST_F(ChunkRadiusChecksTests, unloadEverythingClearsList){
    EXPECT_CALL(*slotManager, destroyChunk(::testing::_)).Times(2);

    chunkRadiusLoader->mLoadedChunks.insert(AV::ChunkRadiusLoader::LoadedChunkData(1, 1));
    chunkRadiusLoader->mLoadedChunks.insert(AV::ChunkRadiusLoader::LoadedChunkData(2, 2));
    chunkRadiusLoader->_unloadEverything();

    ASSERT_EQ(chunkRadiusLoader->mLoadedChunks.size(), 0);
}

TEST_F(ChunkRadiusChecksTests, updatePlayerUnloadsEverythingIf0Radius){
    EXPECT_CALL(*slotManager, destroyChunk(::testing::_)).Times(1);

    AV::WorldSingleton::mPlayerLoadRadius = 0;
    chunkRadiusLoader->mLoadedChunks.insert(AV::ChunkRadiusLoader::LoadedChunkData(1, 1));

    chunkRadiusLoader->_updatePlayer(AV::SlotPosition());

    ASSERT_EQ(chunkRadiusLoader->mLoadedChunks.size(), 0);
}

TEST_F(ChunkRadiusChecksTests, updatePlayerUnloadsStaleChunks){
    AV::SystemSettings::_worldSlotSize = 100;
    AV::WorldSingleton::mPlayerLoadRadius = 100;

    AV::ChunkCoordinate coord(200, 200, AV::WorldSingleton::getCurrentMap());
    //We expect some calls to activate chunk, even though this is testing unloads.
    EXPECT_CALL(*slotManager, activateChunk(::testing::_)).Times(::testing::AtLeast(1));
    //The one loaded chunk should be checked with those coordinates.
    EXPECT_CALL(*slotManager, destroyChunk(coord)).Times(1);

    //Insert some obviously stale chunks.
    chunkRadiusLoader->mLoadedChunks.insert(AV::ChunkRadiusLoader::LoadedChunkData(coord.chunkX(), coord.chunkY()));

    chunkRadiusLoader->_updatePlayer(AV::SlotPosition());

    //We can't assert the size of the list because this call will also load some stuff.
    //We're only interested in the unloads for now though, so expecting destroy chunk is fine.
}

TEST_F(ChunkRadiusChecksTests, updatePlayerLoadsChunks){
    AV::SystemSettings::_worldSlotSize = 100;
    AV::WorldSingleton::mPlayerLoadRadius = 50;
    //If it's at the origin, it should be 4.
    EXPECT_CALL(*slotManager, activateChunk(::testing::_)).Times(4);

    chunkRadiusLoader->_updatePlayer(AV::SlotPosition());

    //Check that there are that many entries in the loaded list.
    ASSERT_EQ(chunkRadiusLoader->mLoadedChunks.size(), 4);
}
