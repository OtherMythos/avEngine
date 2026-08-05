#include "gtest/gtest.h"

#include "WorkerTestScripts.h"

#include "Scripting/ScriptNamespace/ScriptWorkerNamespace.h"
#include "Scripting/Worker/ScriptWorkerManager.h"
#include "System/SystemSetup/SystemSettings.h"
#include "Threading/JobDispatcher.h"

#include <chrono>
#include <squirrel.h>
#include <string>
#include <thread>
#include <vector>

using namespace AV;

/*
End to end, through the real job pool: dispatch really does cross a thread boundary here.
*/

namespace {
    //MERGE NOTE (0.2.0): JobDispatcher::initialise is removed on that branch, where the pool
    //starts lazily on the first dispatchJob. Delete this whole helper when merging.
    void ensureJobPool(){
        static bool initialised = false;
        if(initialised) return;
        //Started once for the whole binary and never shut down. Master's JobDispatcher::shutdown
        //leaves the idle worker queue pointing at deleted workers, so a second initialise would
        //hand jobs to freed memory. 0.2.0 fixes that; there is no need to work around it here.
        JobDispatcher::initialise(SystemSettings::getNumWorkerThreads());
        initialised = true;
    }

    class ScriptWorkerManagerFixture : public ::testing::Test{
    protected:
        void SetUp() override{
            ensureJobPool();
            mainVm = sq_open(1024);
            manager = new ScriptWorkerManager();
        }
        void TearDown() override{
            manager->shutdown();
            delete manager;
            sq_close(mainVm);
        }

        //The fixtures are compiled from source strings, so these tests touch no files at all.
        ScriptWorkerId create(const char* scriptSource, const char* sourceName = "fixture"){
            std::string error;
            const ScriptWorkerId id = manager->createFromBuffer(scriptSource, sourceName, error);
            EXPECT_TRUE(id.valid()) << error;
            return id;
        }

        void pushInput(SQInteger amount){
            sq_newtable(mainVm);
            sq_pushstring(mainVm, "amount", -1);
            sq_pushinteger(mainVm, amount);
            sq_newslot(mainVm, -3, SQFalse);
        }

        //Poll the way a game script would, with a bound so a hang fails rather than wedges.
        bool waitForState(ScriptWorkerId id, ScriptWorker::State target, int timeoutMs = 4000){
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
            while(std::chrono::steady_clock::now() < deadline){
                ScriptWorker* worker = manager->get(id);
                if(!worker) return false;
                if(worker->state() == target) return true;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            return false;
        }

        SQInteger claimedInt(const char* key){
            sq_pushstring(mainVm, key, -1);
            EXPECT_TRUE(SQ_SUCCEEDED(sq_get(mainVm, -2)));
            SQInteger out = 0;
            sq_getinteger(mainVm, -1, &out);
            sq_pop(mainVm, 1);
            return out;
        }

        HSQUIRRELVM mainVm = 0;
        ScriptWorkerManager* manager = 0;
    };
}

TEST_F(ScriptWorkerManagerFixture, aWorkerRunsOnThePoolAndItsResultComesBack){
    const ScriptWorkerId id = create(WorkerTestScripts::COUNTER, "counter");

    pushInput(4);
    std::string error;
    ASSERT_TRUE(manager->get(id)->dispatch(mainVm, -1, error)) << error;
    sq_pop(mainVm, 1);

    ASSERT_TRUE(waitForState(id, ScriptWorker::State::ResultReady));

    ASSERT_TRUE(manager->get(id)->claim(mainVm, error)) << error;
    ASSERT_EQ(sq_gettype(mainVm, -1), OT_TABLE);
    ASSERT_EQ(claimedInt("total"), 4);
    sq_pop(mainVm, 1);
}

TEST_F(ScriptWorkerManagerFixture, stateSurvivesAcrossDispatchesOnThePool){
    const ScriptWorkerId id = create(WorkerTestScripts::COUNTER, "counter");
    std::string error;

    for(int i = 0; i < 3; i++){
        pushInput(2);
        ASSERT_TRUE(manager->get(id)->dispatch(mainVm, -1, error)) << error;
        sq_pop(mainVm, 1);

        ASSERT_TRUE(waitForState(id, ScriptWorker::State::ResultReady));
        ASSERT_TRUE(manager->get(id)->claim(mainVm, error)) << error;

        ASSERT_EQ(claimedInt("runs"), i + 1);
        ASSERT_EQ(claimedInt("total"), (i + 1) * 2);
        sq_pop(mainVm, 1);
    }
}

TEST_F(ScriptWorkerManagerFixture, aHandleGoesStaleTheMomentItIsDestroyed){
    const ScriptWorkerId id = create(WorkerTestScripts::COUNTER, "counter");
    ASSERT_NE(manager->get(id), nullptr);

    manager->destroy(id);
    ASSERT_EQ(manager->get(id), nullptr);

    //Idempotent - a second destroy finds a generation which no longer matches.
    manager->destroy(id);
    ASSERT_EQ(manager->get(id), nullptr);
}

TEST_F(ScriptWorkerManagerFixture, aReusedSlotDoesNotResurrectAnOldHandle){
    const ScriptWorkerId first = create(WorkerTestScripts::COUNTER, "counter");
    manager->destroy(first);

    //Takes the slot the first one freed.
    const ScriptWorkerId second = create(WorkerTestScripts::COUNTER, "counter");
    ASSERT_EQ(first.index, second.index);
    ASSERT_NE(first.generation, second.generation);

    ASSERT_EQ(manager->get(first), nullptr);
    ASSERT_NE(manager->get(second), nullptr);
}

TEST_F(ScriptWorkerManagerFixture, destroyingAWorkerMidRunIsReapedByUpdate){
    const ScriptWorkerId id = create(WorkerTestScripts::SPINNER, "spinner");

    pushInput(0);
    std::string error;
    ASSERT_TRUE(manager->get(id)->dispatch(mainVm, -1, error)) << error;
    sq_pop(mainVm, 1);

    ASSERT_TRUE(waitForState(id, ScriptWorker::State::Running));

    //The vm cannot be closed here - a pool thread is inside it - so the worker is parked until
    //the run notices the cancel.
    manager->destroy(id);
    ASSERT_EQ(manager->get(id), nullptr);
    ASSERT_EQ(manager->liveWorkerCount(), 1u);

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(4);
    while(manager->liveWorkerCount() > 0 && std::chrono::steady_clock::now() < deadline){
        manager->update();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    ASSERT_EQ(manager->liveWorkerCount(), 0u) << "update() never reaped the cancelled worker";
}

TEST_F(ScriptWorkerManagerFixture, shutdownDrainsARunWhichIsStillInFlight){
    const ScriptWorkerId id = create(WorkerTestScripts::SPINNER, "spinner");

    pushInput(0);
    std::string error;
    ASSERT_TRUE(manager->get(id)->dispatch(mainVm, -1, error)) << error;
    sq_pop(mainVm, 1);

    ASSERT_TRUE(waitForState(id, ScriptWorker::State::Running));

    //Cancels, waits for the pool to hand the vm back, then closes it. If the cooperative cancel
    //were broken this would sit on its five second timeout instead.
    const auto start = std::chrono::steady_clock::now();
    manager->shutdown();
    const auto elapsed = std::chrono::steady_clock::now() - start;

    ASSERT_EQ(manager->liveWorkerCount(), 0u);
    ASSERT_LT(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count(), 4000)
        << "shutdown fell through to its drain timeout rather than cancelling cleanly";
}

TEST_F(ScriptWorkerManagerFixture, moreWorkersThanPoolThreadsAllComplete){
    //The pool is two threads by default, so these have to queue behind one another. Nothing may
    //deadlock and every run must land.
    const size_t workerCount = SystemSettings::getMaxScriptWorkers();
    ASSERT_GT(workerCount, SystemSettings::getNumWorkerThreads());

    std::vector<ScriptWorkerId> ids;
    for(size_t i = 0; i < workerCount; i++){
        ids.push_back(create(WorkerTestScripts::COUNTER, "counter"));
    }

    std::string error;
    for(ScriptWorkerId id : ids){
        pushInput(3);
        ASSERT_TRUE(manager->get(id)->dispatch(mainVm, -1, error)) << error;
        sq_pop(mainVm, 1);
    }

    for(ScriptWorkerId id : ids){
        ASSERT_TRUE(waitForState(id, ScriptWorker::State::ResultReady)) << "worker " << id.index << " never finished";
        ASSERT_TRUE(manager->get(id)->claim(mainVm, error)) << error;
        ASSERT_EQ(claimedInt("total"), 3);
        sq_pop(mainVm, 1);
    }
}

TEST_F(ScriptWorkerManagerFixture, creatingMoreWorkersThanTheCapAllowsFails){
    std::vector<ScriptWorkerId> ids;
    for(size_t i = 0; i < SystemSettings::getMaxScriptWorkers(); i++){
        ids.push_back(create(WorkerTestScripts::COUNTER, "counter"));
    }

    std::string error;
    const ScriptWorkerId overflow = manager->createFromBuffer(WorkerTestScripts::COUNTER, "counter", error);
    ASSERT_FALSE(overflow.valid());
    ASSERT_NE(error.find("MaxWorkers"), std::string::npos) << error;
}

TEST_F(ScriptWorkerManagerFixture, creatingAWorkerFromAMissingFileFails){
    std::string error;
    const ScriptWorkerId id = manager->create("/tmp/thisScriptDoesNotExist.nut", error);
    ASSERT_FALSE(id.valid());
    ASSERT_FALSE(error.empty());
    //The failed worker must not have consumed a slot.
    ASSERT_EQ(manager->liveWorkerCount(), 0u);
}

TEST_F(ScriptWorkerManagerFixture, theNamespacePointerFollowsTheManagerLifetime){
    //How script reaches the manager. A call arriving after shutdown has to find nothing rather
    //than a destroyed object.
    ASSERT_EQ(ScriptWorkerNamespace::mManager, manager);

    manager->shutdown();
    ASSERT_EQ(ScriptWorkerNamespace::mManager, nullptr);
}
