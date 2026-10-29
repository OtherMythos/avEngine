#include "gtest/gtest.h"

#include "Threading/JobDispatcher.h"
#include "Threading/Jobs/Job.h"
#include "System/SystemSetup/SystemSettings.h"

#include <atomic>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <vector>
#include "unit/src/TestAccessors.h"

using namespace AV;

namespace {
    //The dispatcher deletes a job once it has run, so these report through shared counters
    //rather than through the object.
    std::atomic<int> gProcessed{0};
    std::atomic<int> gFinished{0};
    std::mutex gMutex;
    std::condition_variable gChanged;

    class CountingJob : public Job{
    public:
        void process() override { gProcessed++; }
        void finish() override {
            { std::lock_guard<std::mutex> lock(gMutex); gFinished++; }
            gChanged.notify_all();
        }
    };

    void resetCounters(){
        gProcessed = 0;
        gFinished = 0;
    }

    bool waitForFinished(int expected, int timeoutMs = 4000){
        std::unique_lock<std::mutex> lock(gMutex);
        return gChanged.wait_for(lock, std::chrono::milliseconds(timeoutMs), [expected]{ return gFinished.load() >= expected; });
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

namespace{
    struct WorkOrder{
        std::mutex mutex;
        std::condition_variable changed;
        bool entered = false, release = false;
        std::vector<int> order;
    };
    class OrderedJob : public Job{
    public:
        OrderedJob(WorkOrder& order, int value) : mOrder(order), mValue(value) {}
        void process() override{
            std::unique_lock<std::mutex> lock(mOrder.mutex);
            if(mValue == 0){
                mOrder.entered = true;
                mOrder.changed.notify_all();
                mOrder.changed.wait(lock, [this]{ return mOrder.release; });
            }
            mOrder.order.push_back(mValue);
            mOrder.changed.notify_all();
        }
        void finish() override {}
    private:
        WorkOrder& mOrder;
        int mValue;
    };
}

TEST(JobDispatcherTests, PriorityPassesQueuedWorkWithoutPreemptingRunningJob){
    const auto previous = TestableSystemSettings::mNumWorkerThreads;
    TestableSystemSettings::mNumWorkerThreads = 1;
    WorkOrder order;
    JobDispatcher::dispatchJob(new OrderedJob(order, 0));
    {
        std::unique_lock<std::mutex> lock(order.mutex);
        EXPECT_TRUE(order.changed.wait_for(lock, std::chrono::seconds(1), [&]{ return order.entered; }));
    }
    JobDispatcher::dispatchJob(new OrderedJob(order, 3));
    JobDispatcher::dispatchJob(new OrderedJob(order, 1), JobDispatcher::Priority::High);
    JobDispatcher::dispatchJob(new OrderedJob(order, 2), JobDispatcher::Priority::High);
    JobDispatcher::dispatchJob(new OrderedJob(order, 4));
    EXPECT_EQ(JobDispatcher::activeWorkerCount(), 1u);
    {
        std::unique_lock<std::mutex> lock(order.mutex);
        EXPECT_TRUE(order.order.empty());
        order.release = true;
        order.changed.notify_all();
        EXPECT_TRUE(order.changed.wait_for(lock, std::chrono::seconds(1), [&]{ return order.order.size() == 5; }));
    }
    JobDispatcher::shutdown();
    TestableSystemSettings::mNumWorkerThreads = previous;
    EXPECT_EQ(order.order, (std::vector<int>{0, 1, 2, 3, 4}));
}
