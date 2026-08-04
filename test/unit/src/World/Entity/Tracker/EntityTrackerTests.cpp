#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "World/Entity/Tracker/EntityTracker.h"
#include "World/Entity/Tracker/EntityTrackerChunk.h"

class EntityTrackerChunkMock : public AV::EntityTrackerChunk{
public:
    EntityTrackerChunkMock() : EntityTrackerChunk() {};

    MOCK_METHOD1(addEntity, void(AV::eId e));
};

//Re-exposes the protected members under test. See EntityTracker.h for why this is a
//subclass rather than a friend: it keeps production headers unaware of test names.
class TestableEntityTracker : public AV::EntityTracker{
public:
    using AV::EntityTracker::ChunkEntry;
    using AV::EntityTracker::mEChunks;
    using AV::EntityTracker::_eChunkExists;
    using AV::EntityTracker::mTrackedEntities;
};

class EntityTrackerTests : public ::testing::Test {
protected:
    TestableEntityTracker* tracker;
public:
    EntityTrackerTests() {
    }

    virtual ~EntityTrackerTests() {
    }

    virtual void SetUp() {
        tracker = new TestableEntityTracker();
    }

    virtual void TearDown() {
        delete tracker;
    }
};

TEST_F(EntityTrackerTests, EChunkExistsReturnsFalse){
    TestableEntityTracker::ChunkEntry e(0, 0);
    ASSERT_FALSE(tracker->_eChunkExists(e));
}

TEST_F(EntityTrackerTests, EChunkExistsReturnsTrue){
    //Value not in list
    TestableEntityTracker::ChunkEntry e(0, 0);
    ASSERT_FALSE(tracker->_eChunkExists(e));

    tracker->mEChunks.insert(std::pair<TestableEntityTracker::ChunkEntry, AV::EntityTrackerChunk*>(e, 0));
    ASSERT_TRUE(tracker->_eChunkExists(e));
}

TEST_F(EntityTrackerTests, trackKnownEntityCreatesChunk){
    ASSERT_EQ(0, tracker->mEChunks.size());

    AV::SlotPosition pos(1, 1);
    AV::eId e;
    tracker->trackKnownEntity(e, pos);

    ASSERT_EQ(1, tracker->mEChunks.size());
}

TEST_F(EntityTrackerTests, trackKnownEntityInsertsIntoChunk){
    AV::SlotPosition pos(1, 1);
    TestableEntityTracker::ChunkEntry e(pos.chunkX(), pos.chunkY());
    EntityTrackerChunkMock chunk;

    AV::eId entity;
    EXPECT_CALL(chunk, addEntity(entity)).Times(1);
    tracker->mEChunks.insert(std::pair<TestableEntityTracker::ChunkEntry, AV::EntityTrackerChunk*>(e, &chunk));

    tracker->trackKnownEntity(entity, pos);

    ASSERT_EQ(tracker->mTrackedEntities, 1);

    tracker->mEChunks.clear();
}
