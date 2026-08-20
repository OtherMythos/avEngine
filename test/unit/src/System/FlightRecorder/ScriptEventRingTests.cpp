#ifdef FLIGHT_RECORDER

#include "gtest/gtest.h"

#include "System/FlightRecorder/ScriptEventRing.h"

using namespace AV;

TEST(ScriptEventRingTests, recordsCallsAndReturns){
    ScriptEventRing ring;
    ring.configure(16);

    ring.pushCall(1, 10, 100);
    ring.pushReturn(11, 200);

    ASSERT_EQ(2u, ring.size());
    ASSERT_EQ(2u, ring.totalEvents());
    ASSERT_EQ(ScriptEventType::Call, ring.at(0).type);
    ASSERT_EQ(1u, ring.at(0).funcIndex);
    ASSERT_EQ(ScriptEventType::Return, ring.at(1).type);
}

TEST(ScriptEventRingTests, tracksStackDepthAcrossNestedCalls){
    ScriptEventRing ring;
    ring.configure(16);

    ring.pushCall(1, 0, 0);
    ring.pushCall(2, 0, 0);
    ring.pushCall(3, 0, 0);
    ASSERT_EQ(3u, ring.stackDepth());

    ring.pushReturn(0, 0);
    ring.pushReturn(0, 0);
    ASSERT_EQ(1u, ring.stackDepth());
}

TEST(ScriptEventRingTests, aReturnWithoutItsCallPopsNothing){
    ScriptEventRing ring;
    ring.configure(16);

    //Collection can begin part way through a call, so the matching call was never seen.
    ring.pushReturn(0, 0);

    ASSERT_EQ(0u, ring.stackDepth());
    ASSERT_EQ(1u, ring.size());
}

TEST(ScriptEventRingTests, deepestStackIsTheHighWaterMark){
    ScriptEventRing ring;
    ring.configure(16);

    ring.pushCall(1, 0, 0);
    ring.pushCall(2, 0, 0);
    ring.pushCall(3, 0, 0);
    ring.pushReturn(0, 0);
    ring.pushReturn(0, 0);
    ring.pushReturn(0, 0);

    //The stack is empty by now, but the frame still reports how deep it got.
    const std::vector<uint32_t> deepest = ring.takeDeepestStack();
    ASSERT_EQ(3u, deepest.size());
    ASSERT_EQ(1u, deepest[0]);
    ASSERT_EQ(3u, deepest[2]);
}

TEST(ScriptEventRingTests, deepestStackResetsToTheStillOpenCalls){
    ScriptEventRing ring;
    ring.configure(16);

    ring.pushCall(1, 0, 0);
    ring.pushCall(2, 0, 0);
    ring.pushReturn(0, 0);
    ring.takeDeepestStack();

    //Frame one closed with func 1 still open, so it is genuinely still the high water mark.
    const std::vector<uint32_t> next = ring.takeDeepestStack();
    ASSERT_EQ(1u, next.size());
    ASSERT_EQ(1u, next[0]);
}

TEST(ScriptEventRingTests, wrapsAroundKeepingTheMostRecentEvents){
    ScriptEventRing ring;
    ring.configure(4);

    for(uint32_t i = 0; i < 10; i++) ring.pushCall(i, 0, i);

    ASSERT_EQ(4u, ring.size());
    ASSERT_EQ(10u, ring.totalEvents());
    ASSERT_EQ(6u, ring.oldestIndex());
    //Absolute index 9 was the last written.
    ASSERT_EQ(9u, ring.at(9).funcIndex);
}

TEST(ScriptEventRingTests, availabilityReportsOverwrittenSlices){
    ScriptEventRing ring;
    ring.configure(4);

    for(uint32_t i = 0; i < 10; i++) ring.pushCall(i, 0, i);

    //A frame recorded early has since been overwritten.
    ASSERT_FALSE(ring.available(0, 4));
    ASSERT_TRUE(ring.available(6, 10));
    //An empty slice is trivially available.
    ASSERT_TRUE(ring.available(3, 3));
}

TEST(ScriptEventRingTests, sliceClampsToWhatIsStillHeld){
    ScriptEventRing ring;
    ring.configure(4);

    for(uint32_t i = 0; i < 10; i++) ring.pushCall(i, 0, i);

    //Asks for more than the ring holds; gets the surviving tail rather than garbage.
    const std::vector<ScriptEvent> events = ring.slice(0, 10);
    ASSERT_EQ(4u, events.size());
    ASSERT_EQ(6u, events.front().funcIndex);
    ASSERT_EQ(9u, events.back().funcIndex);
}

TEST(ScriptEventRingTests, sliceOfAnEmptyRangeIsEmpty){
    ScriptEventRing ring;
    ring.configure(8);
    ring.pushCall(1, 0, 0);

    ASSERT_TRUE(ring.slice(1, 1).empty());
    ASSERT_TRUE(ring.slice(5, 2).empty());
}

TEST(ScriptEventRingTests, aReturnRecordsTheFunctionItCloses){
    ScriptEventRing ring;
    ring.configure(8);

    ring.pushCall(7, 0, 0);
    ring.pushReturn(0, 0);

    ASSERT_EQ(7u, ring.at(1).funcIndex);
}

#endif
