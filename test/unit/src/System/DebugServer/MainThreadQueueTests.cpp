#ifdef DEBUG_SERVER

#include "gtest/gtest.h"

#include "System/DebugServer/MainThreadQueue.h"

#include <thread>
#include <atomic>
#include <chrono>
#include <vector>

using namespace AV;

TEST(MainThreadQueueTests, executeRunsClosureExactlyOnceOnPump){
    MainThreadQueue queue;

    std::atomic<int> runCount{0};

    std::thread caller([&]{
        bool result = queue.execute([&]{ runCount++; }, 2000);
        ASSERT_TRUE(result);
    });

    //Give the caller a moment to enqueue, then service it.
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    queue.pump();

    caller.join();

    //A second pump with nothing queued must not re-run the closure.
    queue.pump();

    ASSERT_EQ(runCount.load(), 1);
}

TEST(MainThreadQueueTests, closureRunsOnPumpingThread){
    MainThreadQueue queue;

    std::thread::id pumpThreadId = std::this_thread::get_id();
    std::atomic<bool> ranOnPumpThread{false};

    std::thread caller([&]{
        queue.execute([&]{
            ranOnPumpThread = (std::this_thread::get_id() == pumpThreadId);
        }, 2000);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    queue.pump();

    caller.join();

    ASSERT_TRUE(ranOnPumpThread.load());
}

TEST(MainThreadQueueTests, executeTimesOutWhenNeverPumped){
    MainThreadQueue queue;

    auto start = std::chrono::steady_clock::now();
    bool result = queue.execute([]{}, 100);
    auto elapsed = std::chrono::steady_clock::now() - start;

    ASSERT_FALSE(result);
    ASSERT_GE(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count(), 100);
}

TEST(MainThreadQueueTests, latePumpAfterTimeoutDoesNotCrash){
    MainThreadQueue queue;

    bool result = queue.execute([]{}, 50);
    ASSERT_FALSE(result);

    //The caller has already given up and unlinked its entry; a subsequent pump must be safe.
    queue.pump();
    SUCCEED();
}

TEST(MainThreadQueueTests, concurrentCallersAllServicedByOnePump){
    MainThreadQueue queue;

    const int numCallers = 16;
    std::atomic<int> successCount{0};
    std::atomic<int> runCount{0};

    std::vector<std::thread> callers;
    for(int i = 0; i < numCallers; i++){
        callers.emplace_back([&]{
            bool result = queue.execute([&]{ runCount++; }, 2000);
            if(result) successCount++;
        });
    }

    //Pump repeatedly until every caller has been serviced.
    while(successCount.load() < numCallers){
        queue.pump();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    for(auto& t : callers) t.join();

    ASSERT_EQ(successCount.load(), numCallers);
    ASSERT_EQ(runCount.load(), numCallers);
}

TEST(MainThreadQueueTests, shutdownWakesBlockedCallers){
    MainThreadQueue queue;

    std::atomic<bool> resultCaptured{false};
    std::atomic<bool> callerResult{true};

    std::thread caller([&]{
        //A long timeout that we expect shutdown to interrupt well before it elapses.
        bool result = queue.execute([]{}, 60000);
        callerResult = result;
        resultCaptured = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    queue.shutdown();

    caller.join();

    ASSERT_TRUE(resultCaptured.load());
    ASSERT_FALSE(callerResult.load());
}

TEST(MainThreadQueueTests, executeFailsFastAfterShutdown){
    MainThreadQueue queue;
    queue.shutdown();

    std::atomic<bool> ran{false};
    auto start = std::chrono::steady_clock::now();
    bool result = queue.execute([&]{ ran = true; }, 5000);
    auto elapsed = std::chrono::steady_clock::now() - start;

    ASSERT_FALSE(result);
    ASSERT_FALSE(ran.load());
    //It must not have blocked for anywhere near the timeout.
    ASSERT_LT(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count(), 1000);
}

#endif
