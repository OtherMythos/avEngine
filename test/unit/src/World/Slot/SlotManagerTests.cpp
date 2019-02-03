#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "World/Slot/SlotManager.h"
#include "World/Slot/Chunk/Chunk.h"
#include "World/Slot/Chunk/ChunkFactory.h"
#include "World/Slot/ChunkCoordinate.h"


class ChunkFactoryMock : public AV::ChunkFactory{
public:
    MOCK_METHOD0(initialise, void());
};

AV::SlotManager* setupSlotManager(){
    return new AV::SlotManager(std::make_shared<ChunkFactoryMock>());
}

TEST(SlotManagerTests, findNextBlankReturnNegativeOne){
    std::unique_ptr<AV::SlotManager> slot;

    int val = slot->_findNextBlank(-1);
    ASSERT_EQ(val, -1);
}

TEST(SlotManagerTests, findNextBlankSearch){
    std::unique_ptr<AV::SlotManager> slot(setupSlotManager());

    int val = slot->_findNextBlank(0);
    ASSERT_EQ(val, 0);

    //Shouldn't actually change the value, so the expected value won't be something like 1.
    val = slot->_findNextBlank(0);
    ASSERT_EQ(val, 0);

    val = slot->_findNextBlank(5);
    ASSERT_EQ(val, 5);

    val = slot->_findNextBlank(AV::SlotManager::_MaxRecipies - 1);
    ASSERT_EQ(val, AV::SlotManager::_MaxRecipies - 1);

    val = slot->_findNextBlank(AV::SlotManager::_MaxRecipies);
    ASSERT_EQ(val, -1);

    val = slot->_findNextBlank(1000000);
    ASSERT_EQ(val, -1);
}

TEST(SlotManagerTests, findNextBlankSearchWithoutAvailability){
    std::unique_ptr<AV::SlotManager> slot(setupSlotManager());

    slot->_recipeContainer[0].slotAvailable = false;
    int val = slot->_findNextBlank(0);
    ASSERT_EQ(val, 1);

    //
    for(int i = 0; i < AV::SlotManager::_MaxRecipies; i++){
        slot->_recipeContainer[i].slotAvailable = false;
    }
    val = slot->_findNextBlank(0);
    ASSERT_EQ(val, -1);
}

TEST(SlotManagerTests, findHighestScoringRecipeTest){
    std::unique_ptr<AV::SlotManager> slot(setupSlotManager());

    //Finds first entry if all scores are 0.
    int val = slot->_findHighestScoringRecipe();
    ASSERT_EQ(val, 0);

    //
    slot->_recipeContainer[5].recipeScore = 10;
    val = slot->_findHighestScoringRecipe();
    ASSERT_EQ(val, 5);

    //
    slot->_recipeContainer[6].recipeScore = 100;
    val = slot->_findHighestScoringRecipe();
    ASSERT_EQ(val, 6);

    //
    slot->_recipeContainer[7].recipeScore = 20;
    val = slot->_findHighestScoringRecipe();
    ASSERT_EQ(val, 6);

    //
    slot->_recipeContainer[3].recipeScore = 2000;
    val = slot->_findHighestScoringRecipe();
    ASSERT_EQ(val, 3);
}

TEST(SlotManagerTests, obtainRecipeEntryTest){
    std::unique_ptr<AV::SlotManager> slot(setupSlotManager());

    int val = slot->_obtainRecipeEntry();
    ASSERT_EQ(val, 0);

    //
    slot->_recipeContainer[0].slotAvailable = false;
    val = slot->_obtainRecipeEntry();
    ASSERT_EQ(val, 1);

    //
    for(int i = 0; i < 6; i++)
        slot->_recipeContainer[i].slotAvailable = false;
    val = slot->_obtainRecipeEntry();
    ASSERT_EQ(val, 6);
}

TEST(SlotManagerTests, obtainRecipeEntryTestListFull){
    std::unique_ptr<AV::SlotManager> slot(setupSlotManager());

    for(int i = 0; i < AV::SlotManager::_MaxRecipies; i++)
        slot->_recipeContainer[i].slotAvailable = false;

    //Looks to replace 0.
    int val = slot->_obtainRecipeEntry();
    ASSERT_EQ(val, 0);

    //
    slot->_recipeContainer[5].recipeScore = 10;
    val = slot->_obtainRecipeEntry();
    ASSERT_EQ(val, 5);

    //
    slot->_recipeContainer[7].recipeScore = 100;
    val = slot->_obtainRecipeEntry();
    ASSERT_EQ(val, 7);
}

TEST(SlotManagerTests, claimRecipeEntryTestClaimsEntries){
    std::unique_ptr<AV::SlotManager> slot(setupSlotManager());

    //Check that calling this function will claim that entry.
    //If called again it should obtain the next available entry.
    int val = slot->_claimRecipeEntry();
    ASSERT_EQ(val, 0);

    val = slot->_claimRecipeEntry();
    ASSERT_EQ(val, 1);

    for(int i = 0; i < 5; i++)
        val = slot->_claimRecipeEntry();

    ASSERT_EQ(val, 6);
}

TEST(SlotManagerTests, claimRecipeEntryTestClaimsOldest){
    std::unique_ptr<AV::SlotManager> slot(setupSlotManager());

    int val = 0;
    for(int i = 0; i < AV::SlotManager::_MaxRecipies; i++)
        val = slot->_claimRecipeEntry();

    //Claim the oldest entry.
    val = slot->_claimRecipeEntry();
    ASSERT_EQ(val, 0);

    //Fake a value.
    slot->_recipeContainer[7].recipeScore = 1000;
    val = slot->_claimRecipeEntry();
    ASSERT_EQ(val, 7);
}

TEST(SlotManagerTests, incrementRecipeScoreTestAllAvailable){
    std::unique_ptr<AV::SlotManager> slot(setupSlotManager());

    //If they're all available then they won't be incremented.
    slot->_incrementRecipeScore();

    bool pass = true;
    for(int i = 0; i < AV::SlotManager::_MaxRecipies; i++){
        if(slot->_recipeContainer[i].recipeScore != 0) pass = false;
    }

    ASSERT_TRUE(pass);
}

TEST(SlotManagerTests, incrementRecipeScoreTestNoneAvailable){
    std::unique_ptr<AV::SlotManager> slot(setupSlotManager());

    for(int i = 0; i < AV::SlotManager::_MaxRecipies; i++){
        slot->_recipeContainer[i].slotAvailable = false;
    }
    slot->_incrementRecipeScore();

    bool pass = true;
    for(int i = 0; i < AV::SlotManager::_MaxRecipies; i++){
        if(slot->_recipeContainer[i].recipeScore != 1) pass = false;
    }

    ASSERT_TRUE(pass);
}

TEST(SlotManagerTests, recipeLoadedTestReturnNegativeOne){
    std::unique_ptr<AV::SlotManager> slot(setupSlotManager());

    //All slots are empty it returns -1.
    int val = slot->_recipeLoaded(AV::ChunkCoordinate());
    ASSERT_EQ(val, -1);

    //None are available, so are all checked.
    for(int i = 0; i < AV::SlotManager::_MaxRecipies; i++){
        slot->_recipeContainer[i].slotAvailable = false;
    }
    val = slot->_recipeLoaded(AV::ChunkCoordinate(420, 69, "Squad fam"));
    ASSERT_EQ(val, -1);
}

TEST(SlotManagerTests, recipeLoadedTestReturnAnEntry){
    std::unique_ptr<AV::SlotManager> slot(setupSlotManager());

    AV::ChunkCoordinate coord(420, 69, "Squad fam");
    slot->_recipeContainer[7].coord = coord;
    slot->_recipeContainer[7].slotAvailable = false;

    int val = slot->_recipeLoaded(coord);
    ASSERT_EQ(val, 7);

    //Really there would never be two entries, but the system should return the first one it finds.
    slot->_recipeContainer[3].coord = coord;
    slot->_recipeContainer[3].slotAvailable = false;
    val = slot->_recipeLoaded(coord);
    ASSERT_EQ(val, 3);
}

TEST(SlotManagerTests, chunkInActivationListTestReturnNegativeOne){
    std::unique_ptr<AV::SlotManager> slot(setupSlotManager());

    AV::ChunkCoordinate coord(1, 1, "Something");
    int val = slot->_chunkInActivationList(coord);
    ASSERT_EQ(val, -1);

    //Check that even though the coord is the same nothing happens.
    slot->_recipeContainer[0].coord = coord;
    val = slot->_chunkInActivationList(coord);
    ASSERT_EQ(val, -1);
}

TEST(SlotManagerTests, chunkInActivationListTestReturnIndex){
    std::unique_ptr<AV::SlotManager> slot(setupSlotManager());

    AV::ChunkCoordinate coord(1, 1, "Something");
    slot->_recipeContainer[0].coord = coord;
    slot->_activationList[0] = true;
    int val = slot->_chunkInActivationList(coord);
    ASSERT_EQ(0, val);

    //Check it returns the first value found.
    slot->_recipeContainer[7].coord = coord;
    slot->_activationList[7] = true;
    val = slot->_chunkInActivationList(coord);
    ASSERT_EQ(0, val);
}

TEST(SlotManagerTests, findChunkTestReturnNull){
    std::unique_ptr<AV::SlotManager> slot(setupSlotManager());

    AV::ChunkCoordinate coord(1, 1, "Something");
    AV::Chunk* val = slot->_findChunk(coord);
    ASSERT_EQ(val, (void*)0);
}

TEST(SlotManagerTests, findChunkTestReturnPointer){
    std::unique_ptr<AV::SlotManager> slot(setupSlotManager());

    AV::ChunkCoordinate coord(1, 1, "Something");
    AV::Chunk* c = (AV::Chunk*)100;
    AV::SlotManager::ChunkEntry entry(coord, c);
    slot->mTotalChunks.push_back(entry);

    AV::Chunk* val = slot->_findChunk(coord);
    ASSERT_EQ(val, c);

    //Make sure it still returns the first one.
    AV::Chunk* s = (AV::Chunk*)200;
    AV::SlotManager::ChunkEntry entryS(coord, s);
    slot->mTotalChunks.push_back(entryS);
    val = slot->_findChunk(coord);
    ASSERT_EQ(val, c);
}

TEST(SlotManagerTests, constructChunkReturnsExistingChunk){
    std::unique_ptr<AV::SlotManager> slot(setupSlotManager());

    AV::ChunkCoordinate coord(1, 1, "Something");
    slot->_recipeContainer[0].coord = coord;
    AV::Chunk* c = (AV::Chunk*)100;
    AV::SlotManager::ChunkEntry entry(coord, c);
    slot->mTotalChunks.push_back(entry);

    AV::Chunk* val = slot->_constructChunk(0, false);
    ASSERT_EQ(val, c);

}

TEST(SlotManagerTests, constructChunkTestConstructsChunk){
    //TODO fill this out with a mock class.
}
