#ifdef DEBUG_SERVER
#include "gtest/gtest.h"
#include "System/DebugServer/MainThreadQueue.h"

#include <future>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

using namespace AV;

namespace{
    bool pumpUntilDone(MainThreadQueue& queue, std::future<bool>& caller){
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(20);
        while(caller.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready){
            if(std::chrono::steady_clock::now() >= deadline){
                queue.shutdown();
                return false;
            }
            queue.pump();
            std::this_thread::yield();
        }
        return caller.get();
    }
}

namespace AV{
    struct MainThreadQueueTestAccess{
        static size_t pending(MainThreadQueue& queue){
            std::lock_guard<std::mutex> lock(queue.mQueueMutex);
            return queue.mQueue.size();
        }
    };
}

TEST(MainThreadQueueTests, timeoutCancelsWorkAlreadyTakenByPump){
    MainThreadQueue queue;
    std::atomic<bool> secondTimedOut{false};
    bool mutationRan = false;
    auto first = std::async(std::launch::async, [&]{
        return queue.execute([&]{
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(15);
            while(!secondTimedOut && std::chrono::steady_clock::now() < deadline) std::this_thread::yield();
        }, 25);
    });
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(20);
    while(MainThreadQueueTestAccess::pending(queue) == 0 && std::chrono::steady_clock::now() < deadline) std::this_thread::yield();
    auto second = std::async(std::launch::async, [&]{
        bool result = queue.execute([&]{ mutationRan = true; }, 5);
        secondTimedOut = !result;
        return result;
    });
    while(MainThreadQueueTestAccess::pending(queue) < 2 && !secondTimedOut && std::chrono::steady_clock::now() < deadline) std::this_thread::yield();
    EXPECT_EQ(MainThreadQueueTestAccess::pending(queue), 2u);
    queue.pump();
    EXPECT_TRUE(first.get());
    EXPECT_FALSE(second.get());
    EXPECT_FALSE(mutationRan);
}

TEST(MainThreadQueueTests, executeRunsClosureExactlyOnceOnPump){
    MainThreadQueue queue;
    int count = 0;
    auto caller = std::async(std::launch::async, [&]{ return queue.execute([&]{ ++count; }, 100); });
    EXPECT_TRUE(pumpUntilDone(queue, caller));
    queue.pump();
    EXPECT_EQ(count, 1);
}

TEST(MainThreadQueueTests, closureRunsOnPumpingThread){
    MainThreadQueue queue;
    const auto mainThread = std::this_thread::get_id();
    bool correctThread = false;
    auto caller = std::async(std::launch::async, [&]{
        return queue.execute([&]{ correctThread = std::this_thread::get_id() == mainThread; }, 100);
    });
    EXPECT_TRUE(pumpUntilDone(queue, caller));
    EXPECT_TRUE(correctThread);
}

TEST(MainThreadQueueTests, executeTimesOutWhenNeverPumped){
    MainThreadQueue queue;
    EXPECT_FALSE(queue.execute([]{}, 1));
}

TEST(MainThreadQueueTests, latePumpNeverExecutesTimedOutMutation){
    MainThreadQueue queue;
    bool ran = false;
    EXPECT_FALSE(queue.execute([&]{ ran = true; }, 1));
    queue.pump();
    EXPECT_FALSE(ran);
}

TEST(MainThreadQueueTests, concurrentCallersAllServiced){
    MainThreadQueue queue;
    int count = 0;
    std::vector<std::future<bool>> callers;
    for(int i = 0; i < 8; ++i){
        callers.emplace_back(std::async(std::launch::async, [&]{ return queue.execute([&]{ ++count; }, 100); }));
    }
    for(auto& caller : callers) EXPECT_TRUE(pumpUntilDone(queue, caller));
    EXPECT_EQ(count, 8);
}

TEST(MainThreadQueueTests, shutdownWakesBlockedCallers){
    MainThreadQueue queue;
    std::atomic<bool> entered{false};
    auto caller = std::async(std::launch::async, [&]{
        entered = true;
        return queue.execute([]{}, 100);
    });
    while(!entered.load()) std::this_thread::yield();
    queue.shutdown();
    EXPECT_FALSE(caller.get());
}

TEST(MainThreadQueueTests, executeFailsFastAfterShutdown){
    MainThreadQueue queue;
    queue.shutdown();
    bool ran = false;
    EXPECT_FALSE(queue.execute([&]{ ran = true; }, 100));
    EXPECT_FALSE(ran);
}
#endif
