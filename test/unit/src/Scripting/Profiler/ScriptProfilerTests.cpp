#ifdef SCRIPT_PROFILER

#include "gtest/gtest.h"

#include "Scripting/Profiler/ScriptProfiler.h"
#include "Scripting/SquirrelHookDispatcher.h"
#include "../ScriptTestHelper.h"

using namespace AV;

namespace{
    uint64_t gClockNs = 0;
    uint64_t testClock(){ return gClockNs; }
    void advanceUs(uint64_t us){ gClockNs += us * 1000; }

    //Squirrel reports the source, the function's opening line and its name on a call.
    void call(const char* name, int declLine = 1, const char* source = "test.nut"){
        ScriptProfiler::_hookForTesting('c', source, declLine, name);
    }
    //Only the event type matters on a return; the profiler closes whatever is on top.
    void ret(){
        ScriptProfiler::_hookForTesting('r', "test.nut", 0, "");
    }
    void line(int number, const char* source = "test.nut"){
        ScriptProfiler::_hookForTesting('l', source, number, "");
    }

    ProfileReport buildFull(){
        ProfileQuery query;
        query.maxFunctions = 0;
        query.includeEdges = true;
        query.includeLines = true;
        query.includeFrames = true;
        query.maxEdges = 0;
        query.maxLines = 0;
        query.maxFrames = 0;

        ProfileReport report;
        ScriptProfiler::buildReport(report, query);
        return report;
    }

    const ProfileReport::Function* functionNamed(const ProfileReport& report, const std::string& name){
        for(const ProfileReport::Function& func : report.functions){
            if(func.name == name) return &func;
        }
        return 0;
    }

    const ProfileReport::Line* lineNumbered(const ProfileReport& report, int number){
        for(const ProfileReport::Line& line : report.hotLines){
            if(line.line == number) return &line;
        }
        return 0;
    }

    const ProfileReport::Edge* edgeBetween(const ProfileReport& report, uint32_t caller, uint32_t callee){
        for(const ProfileReport::Edge& edge : report.edges){
            if(edge.callerId == caller && edge.calleeId == callee) return &edge;
        }
        return 0;
    }
}

class ScriptProfilerTests : public ::testing::Test{
protected:
    void SetUp() override{
        gClockNs = 0;
        ScriptProfiler::_setClockForTesting(testClock);

        ScriptProfilerSettings settings;
        //A null vm skips pinning the interned strings, which is the only part of the
        //profiler that needs one. Everything else is driven through _hookForTesting.
        ScriptProfiler::initialise(0, settings);
    }

    void TearDown() override{
        ScriptProfiler::stop();
        ScriptProfiler::_setClockForTesting(0);
    }
};

TEST_F(ScriptProfilerTests, nestedCallSplitsInclusiveAndExclusive){
    call("outer");
    advanceUs(10);
    call("inner");
    advanceUs(30);
    ret();
    advanceUs(10);
    ret();

    const ProfileReport report = buildFull();

    const ProfileReport::Function* outer = functionNamed(report, "outer");
    const ProfileReport::Function* inner = functionNamed(report, "inner");
    ASSERT_NE(outer, nullptr);
    ASSERT_NE(inner, nullptr);

    //outer was open for the whole 50us but only 20us of it was its own work.
    EXPECT_DOUBLE_EQ(outer->inclusiveMs, 0.05);
    EXPECT_DOUBLE_EQ(outer->exclusiveMs, 0.02);
    EXPECT_DOUBLE_EQ(inner->inclusiveMs, 0.03);
    EXPECT_DOUBLE_EQ(inner->exclusiveMs, 0.03);
}

TEST_F(ScriptProfilerTests, repeatedCallsAggregate){
    for(int i = 0; i < 4; i++){
        call("tick");
        advanceUs(5);
        ret();
        advanceUs(100);
    }

    const ProfileReport report = buildFull();
    const ProfileReport::Function* tick = functionNamed(report, "tick");
    ASSERT_NE(tick, nullptr);

    EXPECT_EQ(tick->callCount, 4u);
    //The 100us between calls is not script time and must not be counted.
    EXPECT_DOUBLE_EQ(tick->inclusiveMs, 0.02);
    EXPECT_DOUBLE_EQ(tick->avgUs, 5.0);
    EXPECT_DOUBLE_EQ(tick->maxUs, 5.0);
    EXPECT_DOUBLE_EQ(tick->minUs, 5.0);
}

TEST_F(ScriptProfilerTests, recursionDoesNotDoubleCountInclusiveTime){
    //Three levels deep, 10us spent at each step in and each step out.
    call("recurse");
    advanceUs(10);
    call("recurse");
    advanceUs(10);
    call("recurse");
    advanceUs(10);
    ret();
    advanceUs(10);
    ret();
    advanceUs(10);
    ret();

    const ProfileReport report = buildFull();
    const ProfileReport::Function* recurse = functionNamed(report, "recurse");
    ASSERT_NE(recurse, nullptr);

    EXPECT_EQ(recurse->callCount, 3u);
    //50us of wall time elapsed. Counting every level would report 120us instead.
    EXPECT_DOUBLE_EQ(recurse->inclusiveMs, 0.05);
    EXPECT_DOUBLE_EQ(recurse->exclusiveMs, 0.05);
}

TEST_F(ScriptProfilerTests, returnWithNothingOpenIsIgnored){
    //What collection starting part way through a call looks like.
    ret();
    ret();

    call("after");
    advanceUs(20);
    ret();

    const ProfileReport report = buildFull();
    const ProfileReport::Function* after = functionNamed(report, "after");
    ASSERT_NE(after, nullptr);
    EXPECT_EQ(after->callCount, 1u);
    EXPECT_DOUBLE_EQ(after->inclusiveMs, 0.02);
}

TEST_F(ScriptProfilerTests, surplusReturnsDoNotCorruptTheStack){
    //Exception unwinding emits a return per open frame, and a mismatch would leave the
    //shadow stack out of step for everything measured afterwards.
    call("a");
    call("b");
    advanceUs(10);
    ret();
    ret();
    ret();
    ret();

    call("later");
    advanceUs(40);
    ret();

    const ProfileReport report = buildFull();
    const ProfileReport::Function* later = functionNamed(report, "later");
    ASSERT_NE(later, nullptr);
    EXPECT_DOUBLE_EQ(later->inclusiveMs, 0.04);

    //Having been closed at the top level, it is a root call rather than a child of 'a'.
    ASSERT_NE(edgeBetween(report, ProfileReport::ROOT_ID, later->id), nullptr);
}

TEST_F(ScriptProfilerTests, edgesRecordWhoCalledWhat){
    call("update");
    for(int i = 0; i < 3; i++){
        call("helper", 20);
        advanceUs(5);
        ret();
    }
    ret();

    const ProfileReport report = buildFull();
    const ProfileReport::Function* update = functionNamed(report, "update");
    const ProfileReport::Function* helper = functionNamed(report, "helper");
    ASSERT_NE(update, nullptr);
    ASSERT_NE(helper, nullptr);

    const ProfileReport::Edge* called = edgeBetween(report, update->id, helper->id);
    ASSERT_NE(called, nullptr);
    EXPECT_EQ(called->callCount, 3u);
    EXPECT_DOUBLE_EQ(called->inclusiveMs, 0.015);

    //update itself was entered from the engine rather than from squirrel.
    const ProfileReport::Edge* fromEngine = edgeBetween(report, ProfileReport::ROOT_ID, update->id);
    ASSERT_NE(fromEngine, nullptr);
    EXPECT_EQ(fromEngine->callCount, 1u);
}

TEST_F(ScriptProfilerTests, lineTimeIsChargedToTheLineThatWasRunning){
    call("work");
    line(10);
    advanceUs(5);
    line(20);
    advanceUs(50);
    ret();

    const ProfileReport report = buildFull();
    const ProfileReport::Line* first = lineNumbered(report, 10);
    const ProfileReport::Line* second = lineNumbered(report, 20);
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_DOUBLE_EQ(first->selfMs, 0.005);
    //Closed by the return rather than by a following line.
    EXPECT_DOUBLE_EQ(second->selfMs, 0.05);
    EXPECT_EQ(first->executions, 1u);
    EXPECT_EQ(second->executions, 1u);
    //Sorted by self time, so the slow line leads.
    EXPECT_EQ(report.hotLines.front().line, 20);
}

TEST_F(ScriptProfilerTests, lineTimeExcludesSquirrelCallees){
    call("caller");
    line(10);
    advanceUs(5);
    //Everything this line goes on to call belongs to the callee, not to line 10.
    call("callee", 50);
    advanceUs(200);
    ret();
    line(20);
    advanceUs(1);
    ret();

    const ProfileReport report = buildFull();
    const ProfileReport::Line* first = lineNumbered(report, 10);
    ASSERT_NE(first, nullptr);
    EXPECT_DOUBLE_EQ(first->selfMs, 0.005);
}

TEST_F(ScriptProfilerTests, lineExecutionsCountEveryRun){
    call("loop");
    for(int i = 0; i < 6; i++){
        line(30);
        advanceUs(2);
    }
    ret();

    const ProfileReport report = buildFull();
    const ProfileReport::Line* body = lineNumbered(report, 30);
    ASSERT_NE(body, nullptr);
    EXPECT_EQ(body->executions, 6u);
}

TEST_F(ScriptProfilerTests, frameTimelineRecordsScriptTimePerFrame){
    call("cheap");
    advanceUs(100);
    ret();
    ScriptProfiler::notifyFrameBoundary();

    call("expensive", 40);
    advanceUs(4000);
    ret();
    ScriptProfiler::notifyFrameBoundary();

    const ProfileReport report = buildFull();
    ASSERT_EQ(report.frames.size(), 2u);
    EXPECT_DOUBLE_EQ(report.frames[0].scriptMs, 0.1);
    EXPECT_DOUBLE_EQ(report.frames[1].scriptMs, 4.0);
    EXPECT_EQ(report.frames[0].calls, 1u);

    //The frame's worst root call is what a spike should be blamed on.
    const ProfileReport::Function* expensive = functionNamed(report, "expensive");
    ASSERT_NE(expensive, nullptr);
    EXPECT_EQ(report.frames[1].topRootFunctionId, expensive->id);
}

TEST_F(ScriptProfilerTests, resetClearsCountersButKeepsFunctionIds){
    call("persistent");
    advanceUs(10);
    ret();

    const ProfileReport before = buildFull();
    const ProfileReport::Function* first = functionNamed(before, "persistent");
    ASSERT_NE(first, nullptr);
    const uint32_t id = first->id;

    ScriptProfiler::reset();

    call("persistent");
    advanceUs(3);
    ret();

    const ProfileReport after = buildFull();
    const ProfileReport::Function* second = functionNamed(after, "persistent");
    ASSERT_NE(second, nullptr);
    //Ids stay stable so a caller can reset and keep drilling into the same function.
    EXPECT_EQ(second->id, id);
    EXPECT_EQ(second->callCount, 1u);
    EXPECT_DOUBLE_EQ(second->inclusiveMs, 0.003);
}

TEST_F(ScriptProfilerTests, stoppedProfilerCollectsNothing){
    ScriptProfiler::stop();

    call("ignored");
    advanceUs(10);
    ret();

    const ProfileReport report = buildFull();
    EXPECT_EQ(functionNamed(report, "ignored"), nullptr);
    EXPECT_EQ(report.totals.totalCalls, 0u);
    EXPECT_FALSE(report.totals.running);
}

TEST_F(ScriptProfilerTests, minCallsFiltersOutQuietFunctions){
    call("busy");
    ret();
    call("busy");
    ret();
    call("quiet", 60);
    ret();

    ProfileQuery query;
    query.minCalls = 2;
    ProfileReport report;
    ScriptProfiler::buildReport(report, query);

    EXPECT_NE(functionNamed(report, "busy"), nullptr);
    EXPECT_EQ(functionNamed(report, "quiet"), nullptr);
}

//Drives the profiler through a real vm and a real script, so the assumptions about what
//squirrel's debug hook actually emits are covered rather than just the bookkeeping.
TEST(ScriptProfilerIntegrationTests, measuresRealSquirrelExecution){
    HSQUIRRELVM vm = ScriptTestHelper::getVM();
    //Line events only exist if the script was compiled with debug info, as the engine's
    //vm is. ScriptTestHelper does not enable it.
    sq_enabledebuginfo(vm, true);

    SquirrelHookDispatcher::initialise(vm);
    ScriptProfilerSettings settings;
    ScriptProfiler::initialise(vm, settings);

    ScriptTestHelper::executeString(
        "function profilerInner(a){ return a * 2; } "
        "function profilerOuter(){ "
        "    local total = 0; "
        "    for(local i = 0; i < 5; i++){ total += profilerInner(i); } "
        "    return total; "
        "} "
        "profilerOuter();");

    const ProfileReport report = buildFull();

    const ProfileReport::Function* inner = functionNamed(report, "profilerInner");
    const ProfileReport::Function* outer = functionNamed(report, "profilerOuter");
    ASSERT_NE(inner, nullptr);
    ASSERT_NE(outer, nullptr);

    EXPECT_EQ(inner->callCount, 5u);
    EXPECT_EQ(outer->callCount, 1u);
    //Inclusive time covers the callees, exclusive does not.
    EXPECT_GE(outer->inclusiveMs, outer->exclusiveMs);

    const ProfileReport::Edge* edge = edgeBetween(report, outer->id, inner->id);
    ASSERT_NE(edge, nullptr);
    EXPECT_EQ(edge->callCount, 5u);

    //The loop body should have run at least once per iteration.
    EXPECT_GT(report.totals.lineExecutions, 5u);

    ScriptProfiler::stop();
    //Leave the shared vm without a hook, so later tests are not measured.
    SquirrelHookDispatcher::shutdown();
}

#endif
