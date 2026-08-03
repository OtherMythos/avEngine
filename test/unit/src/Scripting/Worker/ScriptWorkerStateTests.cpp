#include "gtest/gtest.h"

//Pulled in before the access rewrite below, so the standard library is not compiled with it.
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <random>
#include <squirrel.h>
#include <string>
#include <thread>

//Reaches _publishRun, the half of dispatch which does not queue a job. Same idiom as
//ScriptTestHelper and SystemSetupTests.
#define private public
#include "Scripting/Worker/ScriptWorker.h"
#undef private

#include "WorkerTestScripts.h"

using namespace AV;

/*
Drives ScriptWorker directly, without the job pool, so the state machine and the vm handover can
be tested in isolation - runEntry() is called from a std::thread here, which is exactly what a
pool thread would do to it.

These tests deliberately go through _publishRun rather than dispatch(). dispatch() also queues a
real job, and once any other test in the binary has started the pool that job would run the worker
a second time, concurrently with the runEntry() call below. The pool path is covered end to end by
ScriptWorkerManagerTests instead.
*/

namespace {
    ScriptWorkerId testId(uint16 index = 0){
        ScriptWorkerId id;
        id.index = index;
        id.generation = 1;
        return id;
    }

    //A stand in for the main vm. Bare, since dispatch and claim only need somewhere to copy
    //values to and from.
    class ScriptWorkerFixture : public ::testing::Test{
    protected:
        void SetUp() override { mainVm = sq_open(1024); }
        void TearDown() override{
            if(worker){
                worker->teardown();
                worker.reset();
            }
            sq_close(mainVm);
        }

        void createWorker(const char* scriptSource, const char* sourceName = "fixture"){
            worker = std::make_shared<ScriptWorker>(testId(), 1234);
            std::string error;
            ASSERT_TRUE(worker->prepareFromBuffer(scriptSource, sourceName, error)) << error;
        }

        //Push a table with a single integer member, as an input for dispatch.
        void pushInput(const char* key, SQInteger value){
            sq_newtable(mainVm);
            sq_pushstring(mainVm, key, -1);
            sq_pushinteger(mainVm, value);
            sq_newslot(mainVm, -3, SQFalse);
        }

        SQInteger claimedInt(const char* key){
            sq_pushstring(mainVm, key, -1);
            EXPECT_TRUE(SQ_SUCCEEDED(sq_get(mainVm, -2)));
            SQInteger out = 0;
            sq_getinteger(mainVm, -1, &out);
            sq_pop(mainVm, 1);
            return out;
        }

        //Run the worker on another thread and wait for it to publish a terminal state.
        void runOnAnotherThreadAndWait(){
            std::thread t([this]{ worker->runEntry(); });
            t.join();
        }

        HSQUIRRELVM mainVm = 0;
        ScriptWorkerPtr worker;
    };
}

TEST_F(ScriptWorkerFixture, aFreshlyPreparedWorkerIsIdle){
    createWorker(WorkerTestScripts::COUNTER, "counter");
    ASSERT_EQ(worker->state(), ScriptWorker::State::Idle);
    ASSERT_FALSE(worker->inFlight());
    ASSERT_TRUE(worker->getError().empty());
}

TEST_F(ScriptWorkerFixture, preparingAScriptWithoutARunFunctionFails){
    worker = std::make_shared<ScriptWorker>(testId(), 1);
    std::string error;
    ASSERT_FALSE(worker->prepareFromBuffer(WorkerTestScripts::NO_RUN, "noRun", error));
    ASSERT_NE(error.find("run()"), std::string::npos) << error;
}

TEST_F(ScriptWorkerFixture, preparingAScriptWhichDoesNotCompileFails){
    worker = std::make_shared<ScriptWorker>(testId(), 1);
    std::string error;
    ASSERT_FALSE(worker->prepareFromBuffer(WorkerTestScripts::SYNTAX_ERROR, "broken", error));
    ASSERT_FALSE(error.empty());
    //The name given to prepareFromBuffer is what squirrel reports the failure against, which is
    //the only way to tell which of several in memory scripts went wrong.
    ASSERT_NE(error.find("broken"), std::string::npos) << error;
}

TEST_F(ScriptWorkerFixture, theInputTableArrivesInTheWorkerIntact){
    createWorker(WorkerTestScripts::ECHO, "echo");

    //A nested structure, so this covers more than the flat table the other fixtures take.
    const std::string source = "return { count = 3, name = \"abc\", list = [1, 2, 3], inner = { flag = true } };";
    ASSERT_TRUE(SQ_SUCCEEDED(sq_compilebuffer(mainVm, source.c_str(), source.size(), "test", SQTrue)));
    sq_pushroottable(mainVm);
    ASSERT_TRUE(SQ_SUCCEEDED(sq_call(mainVm, 1, SQTrue, SQTrue)));
    sq_remove(mainVm, -2);

    std::string error;
    ASSERT_TRUE(worker->_publishRun(mainVm, -1, error)) << error;
    sq_pop(mainVm, 1);

    runOnAnotherThreadAndWait();
    ASSERT_EQ(worker->state(), ScriptWorker::State::ResultReady);
    ASSERT_TRUE(worker->claim(mainVm, error)) << error;

    sq_pushstring(mainVm, "received", -1);
    ASSERT_TRUE(SQ_SUCCEEDED(sq_get(mainVm, -2)));
    ASSERT_EQ(sq_gettype(mainVm, -1), OT_TABLE);
    ASSERT_EQ(claimedInt("count"), 3);

    sq_pushstring(mainVm, "inner", -1);
    ASSERT_TRUE(SQ_SUCCEEDED(sq_get(mainVm, -2)));
    ASSERT_EQ(sq_gettype(mainVm, -1), OT_TABLE);
    sq_pop(mainVm, 2);
}

TEST_F(ScriptWorkerFixture, aRunProgressesThroughTheStateMachineAndProducesAResult){
    createWorker(WorkerTestScripts::COUNTER, "counter");
    pushInput("amount", 5);

    std::string error;
    ASSERT_TRUE(worker->_publishRun(mainVm, -1, error)) << error;
    sq_pop(mainVm, 1);

    //The input is published, and Dispatched with it, before any job exists to observe it.
    ASSERT_TRUE(worker->inFlight());

    runOnAnotherThreadAndWait();

    ASSERT_EQ(worker->state(), ScriptWorker::State::ResultReady);
    ASSERT_FALSE(worker->inFlight());
    ASSERT_FLOAT_EQ(worker->progress(), 1.0f);

    ASSERT_TRUE(worker->claim(mainVm, error)) << error;
    ASSERT_EQ(sq_gettype(mainVm, -1), OT_TABLE);
    ASSERT_EQ(claimedInt("total"), 5);
    ASSERT_EQ(claimedInt("echoed"), 5);

    //Claiming returns the worker to idle, ready for the next dispatch.
    ASSERT_EQ(worker->state(), ScriptWorker::State::Idle);
}

TEST_F(ScriptWorkerFixture, theWorkerVmKeepsItsStateBetweenRuns){
    //The whole reason for a persistent worker rather than a fire and forget job.
    createWorker(WorkerTestScripts::COUNTER, "counter");

    std::string error;
    for(int i = 0; i < 3; i++){
        pushInput("amount", 10);
        ASSERT_TRUE(worker->_publishRun(mainVm, -1, error)) << error;
        sq_pop(mainVm, 1);

        runOnAnotherThreadAndWait();
        ASSERT_TRUE(worker->claim(mainVm, error)) << error;

        ASSERT_EQ(claimedInt("runs"), i + 1);
        ASSERT_EQ(claimedInt("total"), (i + 1) * 10);

        sq_pop(mainVm, 1);
    }
}

TEST_F(ScriptWorkerFixture, dispatchIsRefusedUnlessTheWorkerIsIdle){
    createWorker(WorkerTestScripts::COUNTER, "counter");
    pushInput("amount", 1);

    std::string error;
    ASSERT_TRUE(worker->_publishRun(mainVm, -1, error)) << error;

    //A second dispatch while the first is outstanding must be refused, since two jobs against one
    //vm would run squirrel from two threads at once.
    ASSERT_FALSE(worker->_publishRun(mainVm, -1, error));
    ASSERT_FALSE(error.empty());
    ASSERT_EQ(worker->state(), ScriptWorker::State::Dispatched);

    sq_pop(mainVm, 1);
    runOnAnotherThreadAndWait();
}

TEST_F(ScriptWorkerFixture, aFailingRunEndsInFailedWithAnErrorAndClaimsAsNull){
    createWorker(WorkerTestScripts::THROWER, "thrower");
    pushInput("amount", 1);

    std::string error;
    ASSERT_TRUE(worker->_publishRun(mainVm, -1, error)) << error;
    sq_pop(mainVm, 1);

    runOnAnotherThreadAndWait();

    ASSERT_EQ(worker->state(), ScriptWorker::State::Failed);
    ASSERT_FALSE(worker->getError().empty());
    ASSERT_NE(worker->getError().find("deliberate worker failure"), std::string::npos) << worker->getError();

    const SQInteger top = sq_gettop(mainVm);
    ASSERT_TRUE(worker->claim(mainVm, error)) << error;
    ASSERT_EQ(sq_gettop(mainVm), top + 1);
    ASSERT_EQ(sq_gettype(mainVm, -1), OT_NULL);

    //Claiming clears the failure and lets the worker be used again.
    ASSERT_EQ(worker->state(), ScriptWorker::State::Idle);
    ASSERT_TRUE(worker->getError().empty());
}

TEST_F(ScriptWorkerFixture, anInputHoldingAClosureIsRejectedAndLeavesTheWorkerUntouched){
    createWorker(WorkerTestScripts::COUNTER, "counter");

    //{ amount = 1, fn = function(){} } - a closure cannot cross between vms.
    const std::string source = "return { amount = 1, fn = function(){} };";
    ASSERT_TRUE(SQ_SUCCEEDED(sq_compilebuffer(mainVm, source.c_str(), source.size(), "test", SQTrue)));
    sq_pushroottable(mainVm);
    ASSERT_TRUE(SQ_SUCCEEDED(sq_call(mainVm, 1, SQTrue, SQTrue)));
    sq_remove(mainVm, -2);

    const SQInteger mainTop = sq_gettop(mainVm);
    const SQInteger workerTop = sq_gettop(worker->getVM());

    std::string error;
    ASSERT_FALSE(worker->_publishRun(mainVm, -1, error));
    ASSERT_NE(error.find("closure"), std::string::npos) << error;

    //Nothing was queued and neither stack was disturbed.
    ASSERT_EQ(worker->state(), ScriptWorker::State::Idle);
    ASSERT_EQ(sq_gettop(mainVm), mainTop);
    ASSERT_EQ(sq_gettop(worker->getVM()), workerTop);

    sq_pop(mainVm, 1);
}

TEST_F(ScriptWorkerFixture, claimingWithNoResultFails){
    createWorker(WorkerTestScripts::COUNTER, "counter");

    std::string error;
    ASSERT_FALSE(worker->claim(mainVm, error));
    ASSERT_FALSE(error.empty());
}

TEST_F(ScriptWorkerFixture, aRunningScriptObservesRequestCancel){
    createWorker(WorkerTestScripts::SPINNER, "spinner");
    pushInput("amount", 0);

    std::string error;
    ASSERT_TRUE(worker->_publishRun(mainVm, -1, error)) << error;
    sq_pop(mainVm, 1);

    std::thread runner([this]{ worker->runEntry(); });

    //Wait until the script is actually inside run(), then ask it to stop.
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(4);
    while(worker->state() != ScriptWorker::State::Running && std::chrono::steady_clock::now() < deadline){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    ASSERT_EQ(worker->state(), ScriptWorker::State::Running);

    worker->requestCancel();
    runner.join();

    ASSERT_EQ(worker->state(), ScriptWorker::State::ResultReady);

    ASSERT_TRUE(worker->claim(mainVm, error)) << error;
    sq_pushstring(mainVm, "cancelled", -1);
    ASSERT_TRUE(SQ_SUCCEEDED(sq_get(mainVm, -2)));
    SQBool cancelled = SQFalse;
    sq_getbool(mainVm, -1, &cancelled);
    ASSERT_TRUE(cancelled) << "the script left its loop for some reason other than cancellation";
}

TEST_F(ScriptWorkerFixture, eachWorkerHasItsOwnRandomSequence){
    //The engine's _random is rand()/srand(), which is process wide. Two workers must not share.
    ScriptWorkerPtr a = std::make_shared<ScriptWorker>(testId(0), 7);
    ScriptWorkerPtr b = std::make_shared<ScriptWorker>(testId(1), 7);
    ScriptWorkerPtr c = std::make_shared<ScriptWorker>(testId(2), 99);

    const auto draw = [](ScriptWorkerPtr& w){
        std::mt19937::result_type out = 0;
        for(int i = 0; i < 4; i++) out = w->rng()();
        return out;
    };

    //Same seed, same sequence. Different seed, different sequence.
    const auto aVal = draw(a);
    const auto bVal = draw(b);
    const auto cVal = draw(c);
    ASSERT_EQ(aVal, bVal);
    ASSERT_NE(aVal, cVal);

    a->teardown();
    b->teardown();
    c->teardown();
}
