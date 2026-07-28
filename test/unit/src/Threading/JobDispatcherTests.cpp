#include "gtest/gtest.h"

#include "Threading/JobDispatcher.h"
#include "Threading/Jobs/Job.h"
#include "System/SystemSetup/SystemSettings.h"

#include <atomic>
#include <chrono>
#include <thread>

using namespace AV;

namespace {
    //The dispatcher deletes a job once it has run, so these report through shared counters
    //rather than through the object.
    std::atomic<int> gProcessed{0};
    std::atomic<int> gFinished{0};

    class CountingJob : public Job{
    public:
        void process() override { gProcessed++; }
        void finish() override { gFinished++; }
    };

    void resetCounters(){
        gProcessed = 0;
        gFinished = 0;
    }

    bool waitForFinished(int expected, int timeoutMs = 4000){
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
        while(std::chrono::steady_clock::now() < deadline){
            if(gFinished.load() >= expected) return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        return false;
    }
}

TEST(JobDispatcherTests, noThreadsExistUntilAJobIsDispatched){
    resetCounters();

    //Nothing has asked for work, so nothing should have been started.
    ASSERT_EQ(JobDispatcher::activeWorkerCount(), 0u);

    JobDispatcher::shutdown();
    ASSERT_EQ(JobDispatcher::activeWorkerCount(), 0u);
}

TEST(JobDispatcherTests, firstJobStartsTheWorkersAndRuns){
    resetCounters();
    ASSERT_EQ(JobDispatcher::activeWorkerCount(), 0u);

    JobDispatcher::dispatchJob(new CountingJob());

    ASSERT_EQ(JobDispatcher::activeWorkerCount(), SystemSettings::getNumWorkerThreads());

    //The very first job is queued before any thread exists, so this is the case that would
    //strand it if the ordering were the other way around.
    ASSERT_TRUE(waitForFinished(1));
    ASSERT_EQ(gProcessed.load(), 1);

    JobDispatcher::shutdown();
    ASSERT_EQ(JobDispatcher::activeWorkerCount(), 0u);
}

TEST(JobDispatcherTests, poolIsSizedFromTheWorkerThreadSetting){
    resetCounters();
    ASSERT_EQ(JobDispatcher::activeWorkerCount(), 0u);

    JobDispatcher::dispatchJob(new CountingJob());
    ASSERT_EQ(JobDispatcher::activeWorkerCount(), SystemSettings::getNumWorkerThreads());

    //A second job must not create a second pool.
    JobDispatcher::dispatchJob(new CountingJob());
    ASSERT_EQ(JobDispatcher::activeWorkerCount(), SystemSettings::getNumWorkerThreads());

    ASSERT_TRUE(waitForFinished(2));

    JobDispatcher::shutdown();
}

TEST(JobDispatcherTests, workersCanBeStartedAgainAfterAShutdown){
    resetCounters();

    JobDispatcher::dispatchJob(new CountingJob());
    ASSERT_TRUE(waitForFinished(1));
    JobDispatcher::shutdown();
    ASSERT_EQ(JobDispatcher::activeWorkerCount(), 0u);

    //Shutdown used to leave the idle worker queue pointing at deleted workers, which this
    //second round would have run straight into.
    resetCounters();
    JobDispatcher::dispatchJob(new CountingJob());
    ASSERT_EQ(JobDispatcher::activeWorkerCount(), SystemSettings::getNumWorkerThreads());
    ASSERT_TRUE(waitForFinished(1));

    JobDispatcher::shutdown();
}
