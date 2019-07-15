#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "World/Slot/SlotManager.h"
#include "World/Slot/Chunk/Chunk.h"
#include "World/Slot/Chunk/ChunkFactory.h"
#include "World/Slot/ChunkCoordinate.h"
#include "ChunkFactoryMock.h"

class ChunkMock : public AV::Chunk{
public:
    ChunkMock(const AV::ChunkCoordinate &coord, Ogre::SceneManager *sceneManager, Ogre::SceneNode *staticMeshes)
        : Chunk(coord, 0, sceneManager, staticMeshes, AV::PhysicsTypes::EMPTY_CHUNK_ENTRY) {}

    MOCK_METHOD0(activate, void());
    MOCK_METHOD0(deactivate, void());
};

class SlotManagerTests : public ::testing::Test {
private:
    AV::SlotManager* slot;
public:
    SlotManagerTests() {
    }

    virtual ~SlotManagerTests() {
    }

    virtual void SetUp() {
        slot = new AV::SlotManager(std::make_shared<ChunkFactoryMock>());
    }

    virtual void TearDown() {
        delete slot;
    }
};

TEST_F(SlotManagerTests, findNextBlankReturnNegativeOne){
    int val = slot->_findNextBlank(-1);
    ASSERT_EQ(val, -1);
}

TEST_F(SlotManagerTests, findNextBlankSearch){
    int val = slot->_findNextBlank(0);
    ASSERT_EQ(val, 0);

    //Shouldn't actually change the value, so the expected value won't be something like 1.
    val = slot->_findNextBlank(0);
    ASSERT_EQ(val, 0);

    val = slot->_findNextBlank(5);
    ASSERT_EQ(val, 5);

    val = slot->_findNextBlank(AV::SlotManager::mMaxRecipies - 1);
    ASSERT_EQ(val, AV::SlotManager::mMaxRecipies - 1);

    val = slot->_findNextBlank(AV::SlotManager::mMaxRecipies);
    ASSERT_EQ(val, -1);

    val = slot->_findNextBlank(1000000);
    ASSERT_EQ(val, -1);
}

TEST_F(SlotManagerTests, findNextBlankSearchWithoutAvailability){
    slot->_recipeContainer[0].slotAvailable = false;
    int val = slot->_findNextBlank(0);
    ASSERT_EQ(val, 1);

    //
    for(int i = 0; i < AV::SlotManager::mMaxRecipies; i++){
        slot->_recipeContainer[i].slotAvailable = false;
    }
    val = slot->_findNextBlank(0);
    ASSERT_EQ(val, -1);
}

TEST_F(SlotManagerTests, obtainRecipeEntryTest){
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

TEST_F(SlotManagerTests, obtainRecipeEntryTestListFull){
    for(int i = 0; i < AV::SlotManager::mMaxRecipies; i++)
        slot->_recipeContainer[i].slotAvailable = false;

    //Can't find a value, so another function would place it in the queue. In that case this should return -1
    int val = slot->_obtainRecipeEntry();
    ASSERT_EQ(val, -1);

    //These checks are irrelevant now that obtainRecipeEntry will return -1 if it can't find a recipe rather than return one for overriding.
    // slot->_recipeContainer[5].recipeScore = 10;
    // val = slot->_obtainRecipeEntry();
    // ASSERT_EQ(val, 5);

    //
    // slot->_recipeContainer[7].recipeScore = 100;
    // val = slot->_obtainRecipeEntry();
    // ASSERT_EQ(val, 7);
}

TEST_F(SlotManagerTests, claimRecipeEntryTestClaimsEntries){
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

TEST_F(SlotManagerTests, claimRecipeEntryTestClaimsOldest){
    int val = 0;
    for(int i = 0; i < AV::SlotManager::mMaxRecipies; i++)
        val = slot->_claimRecipeEntry();

    //No available recipe slot should be found, so it will return -1.
    val = slot->_claimRecipeEntry();
    ASSERT_EQ(val, -1);

    //Fake a value.
    slot->_recipeContainer[7].recipeScore = 1000;
    val = slot->_claimRecipeEntry();
    ASSERT_EQ(val, -1);
}

TEST_F(SlotManagerTests, incrementRecipeScoreTestAllAvailable){
    //If they're all available then they won't be incremented.
    slot->_incrementRecipeScore();

    bool pass = true;
    for(int i = 0; i < AV::SlotManager::mMaxRecipies; i++){
        if(slot->_recipeContainer[i].recipeScore != 0) pass = false;
    }

    ASSERT_TRUE(pass);
}

TEST_F(SlotManagerTests, incrementRecipeScoreTestNoneAvailable){
    for(int i = 0; i < AV::SlotManager::mMaxRecipies; i++){
        slot->_recipeContainer[i].slotAvailable = false;
    }
    slot->_incrementRecipeScore();

    bool pass = true;
    for(int i = 0; i < AV::SlotManager::mMaxRecipies; i++){
        if(slot->_recipeContainer[i].recipeScore != 1) pass = false;
    }

    ASSERT_TRUE(pass);
}

TEST_F(SlotManagerTests, recipeLoadedTestReturnNegativeOne){
    //All slots are empty it returns -1.
    int val = slot->_recipeLoaded(AV::ChunkCoordinate());
    ASSERT_EQ(val, -1);

    //None are available, so are all checked.
    for(int i = 0; i < AV::SlotManager::mMaxRecipies; i++){
        slot->_recipeContainer[i].slotAvailable = false;
    }
    val = slot->_recipeLoaded(AV::ChunkCoordinate(420, 69, "Squad fam"));
    ASSERT_EQ(val, -1);
}

TEST_F(SlotManagerTests, recipeLoadedTestReturnAnEntry){
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

TEST_F(SlotManagerTests, chunkInActivationListTestReturnNegativeOne){
    AV::ChunkCoordinate coord(1, 1, "Something");
    int val = slot->_chunkInActivationList(coord);
    ASSERT_EQ(val, -1);

    //Check that even though the coord is the same nothing happens.
    slot->_recipeContainer[0].coord = coord;
    val = slot->_chunkInActivationList(coord);
    ASSERT_EQ(val, -1);
}

TEST_F(SlotManagerTests, chunkInActivationListTestReturnIndex){
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

TEST_F(SlotManagerTests, findChunkTestReturnNull){
    AV::ChunkCoordinate coord(1, 1, "Something");
    AV::Chunk* val = slot->_findChunk(coord);
    ASSERT_EQ(val, (void*)0);
}

TEST_F(SlotManagerTests, findChunkTestReturnPointer){
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

TEST_F(SlotManagerTests, constructChunkReturnsExistingChunk){
    AV::ChunkCoordinate coord(1, 1, "Something");
    slot->_recipeContainer[0].coord = coord;
    AV::Chunk* c = (AV::Chunk*)100;
    AV::SlotManager::ChunkEntry entry(coord, c);
    slot->mTotalChunks.push_back(entry);

    AV::Chunk* val = slot->_constructChunk(0, false);
    ASSERT_EQ(val, c);

}

TEST_F(SlotManagerTests, requestInQueueReturnsFalseIfEmpty){
    AV::ChunkCoordinate coord(1, 1, "Something");

    auto val = slot->_requestInQueue(coord);
    ASSERT_EQ(val, slot->queuedEntries.end());
}

TEST_F(SlotManagerTests, requestInQueueReturnsTrueIfContained){
    AV::ChunkCoordinate coord(1, 1, "Something");

    slot->queuedEntries.push_back(AV::SlotManager::QueueEntry(coord, AV::SlotManager::QueuedRecipeType::RecipeTypeConstruct));
    auto val = slot->_requestInQueue(coord);
    ASSERT_NE(val, slot->queuedEntries.end());

    //Get rid of it and check it stays false.
    slot->queuedEntries.pop_front();
    val = slot->_requestInQueue(coord);
    ASSERT_EQ(val, slot->queuedEntries.end());
}

TEST_F(SlotManagerTests, requestInQueueCheckDeletion){
    //Create them
    for(int i = 0; i < 10; i++){
        AV::ChunkCoordinate coord(i, i, "Something");
        slot->queuedEntries.push_back(AV::SlotManager::QueueEntry(coord, AV::SlotManager::QueuedRecipeType::RecipeTypeConstruct));
    }
    AV::ChunkCoordinate coord(5, 5, "Something");
    auto val = slot->_requestInQueue(coord);
    //Check it's there
    ASSERT_NE(val, slot->queuedEntries.end());

    //Delete that value.
    auto it = slot->queuedEntries.begin();
    while(it != slot->queuedEntries.end()){
        if((*it).first == coord){
            slot->queuedEntries.erase(it);
        }
        it++;
    }
    val = slot->_requestInQueue(coord);
    ASSERT_EQ(val, slot->queuedEntries.end());

}

TEST_F(SlotManagerTests, activateChunkWhenAllPendingCausesQueuePush){
    for(int i = 0; i < AV::SlotManager::mMaxRecipies; i++){
        //Set to a pending recipe.
        slot->_recipeContainer[i].slotAvailable = false;
        slot->_recipeContainer[i].recipeReady = false;
    }
    AV::ChunkCoordinate coord(5, 5, "Something");
    slot->activateChunk(coord);
    //Check the value has been pushed into the queue.
    ASSERT_EQ(slot->queuedEntries.front().first, coord);
}

TEST_F(SlotManagerTests, activateChunkWhenAllPendingChecksQueue){
    for(int i = 0; i < AV::SlotManager::mMaxRecipies; i++){
        //Set to a pending recipe.
        slot->_recipeContainer[i].slotAvailable = false;
        slot->_recipeContainer[i].recipeReady = false;
    }
    AV::ChunkCoordinate coord(5, 5, "Something");
    slot->queuedEntries.push_back(AV::SlotManager::QueueEntry(coord, AV::SlotManager::QueuedRecipeType::RecipeTypeConstruct));
    slot->activateChunk(coord);
    //Check the value hasn't been pushed into the queue again.
    ASSERT_EQ(slot->queuedEntries.size(), 1);
}

TEST_F(SlotManagerTests, activateChunkWhenAllPendingSetsQueueTypeActivate){
    for(int i = 0; i < AV::SlotManager::mMaxRecipies; i++){
        //Set to a pending recipe.
        slot->_recipeContainer[i].slotAvailable = false;
        slot->_recipeContainer[i].recipeReady = false;
    }
    AV::ChunkCoordinate coord(5, 5, "Something");
    slot->queuedEntries.push_back(AV::SlotManager::QueueEntry(coord, AV::SlotManager::QueuedRecipeType::RecipeTypeConstruct));
    slot->activateChunk(coord);

    //Check the function set this item in the queue as an activate request.
    ASSERT_EQ(slot->queuedEntries.front().second, AV::SlotManager::QueuedRecipeType::RecipeTypeActivate);
}


TEST_F(SlotManagerTests, constructChunkWhenAllPendingCausesQueuePush){
    for(int i = 0; i < AV::SlotManager::mMaxRecipies; i++){
        slot->_recipeContainer[i].slotAvailable = false;
        slot->_recipeContainer[i].recipeReady = false;
    }
    AV::ChunkCoordinate coord(5, 5, "Something");
    slot->constructChunk(coord);
    //Check the value has been pushed into the queue.
    ASSERT_EQ(slot->queuedEntries.front().first, coord);
    //Check it's a construct command.
    ASSERT_EQ(slot->queuedEntries.front().second, AV::SlotManager::QueuedRecipeType::RecipeTypeConstruct);
}

TEST_F(SlotManagerTests, constructChunkWhenAllPendingChecksQueue){
    for(int i = 0; i < AV::SlotManager::mMaxRecipies; i++){
        //Set to a pending recipe.
        slot->_recipeContainer[i].slotAvailable = false;
        slot->_recipeContainer[i].recipeReady = false;
    }
    AV::ChunkCoordinate coord(5, 5, "Something");
    slot->queuedEntries.push_back(AV::SlotManager::QueueEntry(coord, AV::SlotManager::QueuedRecipeType::RecipeTypeConstruct));
    slot->activateChunk(coord);
    //Check the value hasn't been pushed into the queue again.
    ASSERT_EQ(slot->queuedEntries.size(), 1);
}

TEST_F(SlotManagerTests, constructChunkWhenAllPendingSetsQueueTypeConstruct){
    for(int i = 0; i < AV::SlotManager::mMaxRecipies; i++){
        //Set to a pending recipe.
        slot->_recipeContainer[i].slotAvailable = false;
        slot->_recipeContainer[i].recipeReady = false;
    }
    AV::ChunkCoordinate coord(5, 5, "Something");
    slot->queuedEntries.push_back(AV::SlotManager::QueueEntry(coord, AV::SlotManager::QueuedRecipeType::RecipeTypeActivate));
    slot->constructChunk(coord);

    //Check the function set this item in the queue as an activate request.
    ASSERT_EQ(slot->queuedEntries.front().second, AV::SlotManager::QueuedRecipeType::RecipeTypeConstruct);
}

TEST_F(SlotManagerTests, determineReplacementIndexReturnsMinusOneWhenAllPending){
    for(int i = 0; i < AV::SlotManager::mMaxRecipies; i++){
        //Set to a pending recipe.
        slot->_recipeContainer[i].slotAvailable = false;
        slot->_recipeContainer[i].recipeReady = false;
    }
    int val = slot->_determineReplacementIndex();
    ASSERT_EQ(-1, val);
}

TEST_F(SlotManagerTests, determineReplacementIndexReturnsEntryWhenOneNotPending){
    for(int i = 0; i < AV::SlotManager::mMaxRecipies; i++){
        slot->_recipeContainer[i].slotAvailable = false;
        slot->_recipeContainer[i].recipeReady = false;
    }
    //Recipe 3 is no longer pending.
    slot->_recipeContainer[3].recipeReady = true;
    slot->_recipeContainer[3].recipeScore = 0;

    int val = slot->_determineReplacementIndex();
    ASSERT_EQ(3, val);

    slot->_recipeContainer[3].recipeScore = 100;

    val = slot->_determineReplacementIndex();
    ASSERT_EQ(3, val);
}

TEST_F(SlotManagerTests, determineReplacementIndexReturnsFirstEntryFound){
    for(int i = 0; i < AV::SlotManager::mMaxRecipies; i++){
        slot->_recipeContainer[i].slotAvailable = false;
        slot->_recipeContainer[i].recipeReady = false;
    }
    //Multiple not pending
    slot->_recipeContainer[3].recipeReady = true;
    slot->_recipeContainer[3].recipeScore = 0;

    slot->_recipeContainer[7].recipeReady = true;
    slot->_recipeContainer[7].recipeScore = 0;

    int val = slot->_determineReplacementIndex();
    ASSERT_EQ(3, val);
}

TEST_F(SlotManagerTests, determineReplacementIndexReturnsHighestScoringValue){
    for(int i = 0; i < AV::SlotManager::mMaxRecipies; i++){
        slot->_recipeContainer[i].slotAvailable = false;
        slot->_recipeContainer[i].recipeReady = false;
    }
    slot->_recipeContainer[3].recipeReady = true;
    slot->_recipeContainer[3].recipeScore = 10;

    slot->_recipeContainer[7].recipeReady = true;
    slot->_recipeContainer[7].recipeScore = 100;

    slot->_recipeContainer[8].recipeReady = true;
    slot->_recipeContainer[8].recipeScore = 50;

    int val = slot->_determineReplacementIndex();
    ASSERT_EQ(7, val);
}

TEST_F(SlotManagerTests, loadRecipeStartsRecipeJob){
    std::shared_ptr<ChunkFactoryMock> factoryShared = std::make_shared<ChunkFactoryMock>();
    EXPECT_CALL(*factoryShared, startRecipeJob(::testing::_, ::testing::_))
    .Times(::testing::Exactly(1));

    std::unique_ptr<AV::SlotManager> slot(new AV::SlotManager(factoryShared));

    AV::ChunkCoordinate coord(5, 5, "Something");
    slot->_loadRecipe(coord, AV::SlotManager::QueuedRecipeType::RecipeTypeConstruct);
}

TEST_F(SlotManagerTests, loadRecipeQueuesRequestWhenPending){
    std::shared_ptr<ChunkFactoryMock> factoryShared = std::make_shared<ChunkFactoryMock>();
    EXPECT_CALL(*factoryShared, startRecipeJob(::testing::_, ::testing::_))
    .Times(::testing::Exactly(0));

    std::unique_ptr<AV::SlotManager> slot(new AV::SlotManager(factoryShared));

    //Make all recipies pending.
    for(int i = 0; i < AV::SlotManager::mMaxRecipies; i++){
        slot->_recipeContainer[i].slotAvailable = false;
        slot->_recipeContainer[i].recipeReady = false;
    }

    AV::ChunkCoordinate coord(5, 5, "Something");
    int val = slot->_loadRecipe(coord, AV::SlotManager::QueuedRecipeType::RecipeTypeConstruct);

    ASSERT_EQ(slot->queuedEntries.size(), 1);
    ASSERT_EQ(val, -1);
}

TEST_F(SlotManagerTests, handleChunkRequestActivatesChunkIfChunkExists){
    AV::ChunkCoordinate coord(5, 5, "Something");
    std::unique_ptr<ChunkMock> cMock(new ChunkMock(coord, 0, 0));
    EXPECT_CALL(*cMock, activate())
    .Times(::testing::Exactly(1));

    slot->mTotalChunks.push_back(AV::SlotManager::ChunkEntry(coord, cMock.get()));

    slot->_handleChunkRequest(coord, true);
}

TEST_F(SlotManagerTests, handleChunkRequestDoesntActivateChunkIfConstruction){
    AV::ChunkCoordinate coord(5, 5, "Something");
    std::unique_ptr<ChunkMock> cMock(new ChunkMock(coord, 0, 0));
    EXPECT_CALL(*cMock, activate())
    .Times(::testing::Exactly(0));

    slot->mTotalChunks.push_back(AV::SlotManager::ChunkEntry(coord, cMock.get()));

    slot->_handleChunkRequest(coord, false);
}

TEST_F(SlotManagerTests, handleChunkRequestCreatesItemInQueue){
    //Make all recipies pending.
    for(int i = 0; i < AV::SlotManager::mMaxRecipies; i++){
        slot->_recipeContainer[i].slotAvailable = false;
        slot->_recipeContainer[i].recipeReady = false;
    }
    AV::ChunkCoordinate coord(5, 5, "Something");

    slot->_handleChunkRequest(coord, true);
    ASSERT_EQ(slot->queuedEntries.size(), 1);
}

TEST_F(SlotManagerTests, handleChunkRequestChangesQueuedConstructionType){
    //Make all recipies pending.
    for(int i = 0; i < AV::SlotManager::mMaxRecipies; i++){
        slot->_recipeContainer[i].slotAvailable = false;
        slot->_recipeContainer[i].recipeReady = false;
    }
    AV::ChunkCoordinate coord(5, 5, "Something");

    slot->queuedEntries.push_back(AV::SlotManager::QueueEntry(coord, AV::SlotManager::QueuedRecipeType::RecipeTypeConstruct));

    slot->_handleChunkRequest(coord, true);
    ASSERT_EQ(slot->queuedEntries.front().second, AV::SlotManager::QueuedRecipeType::RecipeTypeActivate);
}

TEST_F(SlotManagerTests, handleChunkRequestIncreasesUpdateCountOnRecipeLoad){
    std::shared_ptr<ChunkFactoryMock> factoryShared = std::make_shared<ChunkFactoryMock>();
    EXPECT_CALL(*factoryShared, startRecipeJob(::testing::_, ::testing::_))
    .Times(::testing::Exactly(1));

    std::unique_ptr<AV::SlotManager> slot(new AV::SlotManager(factoryShared));

    for(int i = 0; i < AV::SlotManager::mMaxRecipies; i++){
        slot->_recipeContainer[i].slotAvailable = false;
        slot->_recipeContainer[i].recipeReady = false;
    }
    //Make one not pending.
    slot->_recipeContainer[2].recipeReady = true;
    AV::ChunkCoordinate coord(5, 5, "Something");

    slot->_handleChunkRequest(coord, true);
    ASSERT_EQ(slot->_updateNeededCount, 1);
}

TEST_F(SlotManagerTests, handleChunkRequestSetsChunkValueIfNotReady){
    //Checks whether the function sets the value in the activation and construction lists.
    AV::ChunkCoordinate coord(5, 5, "Something");

    slot->_recipeContainer[2].coord = coord;
    slot->_recipeContainer[2].recipeReady = false;
    slot->_recipeContainer[2].slotAvailable = false;

    slot->_activationList[2] = false;
    slot->_constructionList[2] = false;

    slot->_handleChunkRequest(coord, true);

    ASSERT_TRUE(slot->_activationList[2]);
    ASSERT_TRUE(slot->_constructionList[2]);
    ASSERT_EQ(slot->_updateNeededCount, 1);

    //
    slot->_activationList[2] = false;
    slot->_constructionList[2] = false;

    slot->_handleChunkRequest(coord, false);
    ASSERT_FALSE(slot->_activationList[2]);
    ASSERT_TRUE(slot->_constructionList[2]);
    ASSERT_EQ(slot->_updateNeededCount, 2);
}

//SetCurrentMap
//SetOrigin
