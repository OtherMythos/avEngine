#ifdef FLIGHT_RECORDER

#include "gtest/gtest.h"

#include "System/FlightRecorder/FrameRing.h"

using namespace AV;

namespace{
    //A record whose pixel payload is a known size, so the byte budget can be reasoned about.
    FrameRecord recordOf(uint64_t frameNumber, size_t bytes){
        FrameRecord record;
        record.frameNumber = frameNumber;
        record.frame.width = 1;
        record.frame.height = static_cast<uint32_t>(bytes / 3);
        record.frame.rgb.resize(bytes);
        return record;
    }
}

TEST(FrameRingTests, startsEmpty){
    FrameRing ring;
    ring.configure(4, 1024);

    ASSERT_TRUE(ring.empty());
    ASSERT_EQ(0u, ring.size());
    ASSERT_EQ(0u, ring.totalBytes());
}

TEST(FrameRingTests, holdsFramesInChronologicalOrder){
    FrameRing ring;
    ring.configure(4, 1024);

    for(uint64_t i = 0; i < 3; i++) ring.push(recordOf(i, 3));

    ASSERT_EQ(3u, ring.size());
    ASSERT_EQ(0u, ring.at(0).frameNumber);
    ASSERT_EQ(2u, ring.at(2).frameNumber);
}

TEST(FrameRingTests, evictsOldestPastCapacity){
    FrameRing ring;
    ring.configure(3, 100000);

    for(uint64_t i = 0; i < 10; i++) ring.push(recordOf(i, 3));

    ASSERT_EQ(3u, ring.size());
    //The three most recent survive, oldest first.
    ASSERT_EQ(7u, ring.at(0).frameNumber);
    ASSERT_EQ(9u, ring.at(2).frameNumber);
}

TEST(FrameRingTests, evictsOldestPastByteBudget){
    FrameRing ring;
    //Capacity is generous, so the byte budget is what binds.
    ring.configure(100, 30);

    for(uint64_t i = 0; i < 10; i++) ring.push(recordOf(i, 9));

    ASSERT_LE(ring.totalBytes(), 30u);
    ASSERT_EQ(3u, ring.size());
    ASSERT_EQ(9u, ring.at(ring.size() - 1).frameNumber);
}

TEST(FrameRingTests, keepsAFrameLargerThanTheWholeBudget){
    FrameRing ring;
    ring.configure(10, 16);

    //Refusing it would leave the ring permanently empty, which is worse than overshooting.
    ring.push(recordOf(1, 300));

    ASSERT_EQ(1u, ring.size());
    ASSERT_EQ(1u, ring.at(0).frameNumber);
}

TEST(FrameRingTests, takeAllEmptiesTheRingAndPreservesOrder){
    FrameRing ring;
    ring.configure(8, 100000);

    for(uint64_t i = 0; i < 4; i++) ring.push(recordOf(i, 3));

    std::vector<FrameRecord> taken = ring.takeAll();

    ASSERT_EQ(4u, taken.size());
    ASSERT_EQ(0u, taken.front().frameNumber);
    ASSERT_EQ(3u, taken.back().frameNumber);

    //A capture must leave the ring ready to record again immediately.
    ASSERT_TRUE(ring.empty());
    ASSERT_EQ(0u, ring.totalBytes());
}

TEST(FrameRingTests, takeAllMovesThePixelsRatherThanCopyingThem){
    FrameRing ring;
    ring.configure(4, 100000);
    ring.push(recordOf(1, 30));

    std::vector<FrameRecord> taken = ring.takeAll();

    ASSERT_EQ(30u, taken.front().frame.rgb.size());
}

TEST(FrameRingTests, reconfiguringDropsWhatWasHeld){
    FrameRing ring;
    ring.configure(4, 100000);
    ring.push(recordOf(1, 3));

    ring.configure(8, 100000);

    ASSERT_TRUE(ring.empty());
    ASSERT_EQ(8u, ring.capacity());
}

#endif
