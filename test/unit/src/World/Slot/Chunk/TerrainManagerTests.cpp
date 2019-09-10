#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "World/Slot/Chunk/TerrainManager.h"
#include "World/Slot/Chunk/Terrain/Terrain.h"

class TerrainManagerTests : public ::testing::Test {
private:
    AV::TerrainManager* terrainManager;
    AV::Terrain* dummyTerrain; //A dummy terrain incase one is needed. <3
public:
    TerrainManagerTests() {
    }

    virtual ~TerrainManagerTests() {
    }

    virtual void SetUp(){
        terrainManager = new AV::TerrainManager();
        dummyTerrain = new AV::Terrain();
    }

    virtual void TearDown(){
        //Some of the tests might have left the dummy terrain in the list. But we want to guarantee it's deleted, so do this here.
        terrainManager->inUseTerrains.erase(dummyTerrain);
        terrainManager->availableTerrains.erase(dummyTerrain);

        delete terrainManager;
        delete dummyTerrain;
    }
};

TEST_F(TerrainManagerTests, requestTerrainReturnsNewTerrain){
    AV::Terrain* t = terrainManager->requestTerrain();

    ASSERT_TRUE(t);
    ASSERT_EQ(terrainManager->availableTerrains.size(), 0);
    ASSERT_EQ(terrainManager->inUseTerrains.size(), 1);
}

TEST_F(TerrainManagerTests, requestTerrainReturnsAvailableTerrain){
    terrainManager->availableTerrains.insert(dummyTerrain);

    ASSERT_EQ(terrainManager->availableTerrains.size(), 1);
    ASSERT_EQ(terrainManager->inUseTerrains.size(), 0);

    AV::Terrain* t = terrainManager->requestTerrain();
    ASSERT_EQ(dummyTerrain, t);

    ASSERT_EQ(terrainManager->availableTerrains.size(), 0);
    ASSERT_EQ(terrainManager->inUseTerrains.size(), 1);
}

TEST_F(TerrainManagerTests, requestTerrainRemovesAndInserts){
    //Create 10 dummy terrains. When we request one it should take only one terrain from this list, and at that point that pointer should be removed from the available list.
    const int dummyCount = 10;
    for(size_t i = 0; i < dummyCount; i++){
        AV::Terrain* t = (AV::Terrain*)i;
        terrainManager->availableTerrains.insert(t);
    }

    ASSERT_EQ(terrainManager->inUseTerrains.size(), 0);
    ASSERT_EQ(terrainManager->availableTerrains.size(), dummyCount);

    AV::Terrain* t = terrainManager->requestTerrain();
    ASSERT_EQ(terrainManager->availableTerrains.size(), dummyCount - 1);
    ASSERT_EQ(terrainManager->inUseTerrains.size(), 1);

    ASSERT_EQ( *(terrainManager->inUseTerrains.begin()) , t);
    ASSERT_EQ( terrainManager->availableTerrains.find(t) , terrainManager->availableTerrains.end()); //The returned pointer should no longer be in the old list.

    terrainManager->availableTerrains.clear();
    terrainManager->inUseTerrains.clear();
}

//TODO releasing terrain
