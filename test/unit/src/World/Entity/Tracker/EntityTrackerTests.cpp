#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "World/Entity/Tracker/EntityTracker.h"
#include "World/Entity/Tracker/EntityTrackerChunk.h"

class EntityTrackerChunkMock : public AV::EntityTrackerChunk{
public:
    EntityTrackerChunkMock() : EntityTrackerChunk() {};

    MOCK_METHOD1(addEntity, void(AV::eId e));
};

TEST(EntityTrackerTests, EChunkExistsReturnsFalse){
    std::unique_ptr<AV::EntityTracker> tracker(new AV::EntityTracker());
    
    //Value not in list
    AV::EntityTracker::ChunkEntry e(0, 0);
    ASSERT_FALSE(tracker->_eChunkExists(e));
}

TEST(EntityTrackerTests, EChunkExistsReturnsTrue){
    std::unique_ptr<AV::EntityTracker> tracker(new AV::EntityTracker());
    
    //Value not in list
    AV::EntityTracker::ChunkEntry e(0, 0);
    ASSERT_FALSE(tracker->_eChunkExists(e));
    
    tracker->mEChunks.insert(std::pair<AV::EntityTracker::ChunkEntry, AV::EntityTrackerChunk*>(e, 0));
    ASSERT_TRUE(tracker->_eChunkExists(e));
}

TEST(EntityTrackerTests, trackKnownEntityCreatesChunk){
    std::unique_ptr<AV::EntityTracker> tracker(new AV::EntityTracker());
    
    ASSERT_EQ(0, tracker->mEChunks.size());
    
    AV::SlotPosition pos(1, 1);
    AV::eId e;
    tracker->trackKnownEntity(e, pos);
    
    ASSERT_EQ(1, tracker->mEChunks.size());
}

TEST(EntityTrackerTests, trackKnownEntityInsertsIntoChunk){
    std::unique_ptr<AV::EntityTracker> tracker(new AV::EntityTracker());
    
    AV::SlotPosition pos(1, 1);
    AV::EntityTracker::ChunkEntry e(pos.chunkX(), pos.chunkY());
    EntityTrackerChunkMock chunk;
    
    AV::eId entity;
    EXPECT_CALL(chunk, addEntity(entity)).Times(1);
    tracker->mEChunks.insert(std::pair<AV::EntityTracker::ChunkEntry, AV::EntityTrackerChunk*>(e, &chunk));
    
    tracker->trackKnownEntity(entity, pos);
    
    ASSERT_EQ(tracker->mTrackedEntities, 1);
}
