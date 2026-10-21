#ifdef DEBUG_SERVER
#include "gtest/gtest.h"
#include "System/DebugServer/MainThreadQueue.h"

#include <future>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

using namespace AV;

namespace{
    bool pumpUntilDone(MainThreadQueue& queue, std::future<bool>& caller){
        //Only a guard against a hang. CI's macOS runners can take 20ms+ to wake a thread, so keep it generous.
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
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
        typedef std::shared_ptr<MainThreadQueue::QueryEntry> EntryPtr;

        static size_t pending(MainThreadQueue& queue){
            std::lock_guard<std::mutex> lock(queue.mQueueMutex);
            return queue.mQueue.size();
        }
        static EntryPtr pendingAt(MainThreadQueue& queue, size_t idx){
            std::lock_guard<std::mutex> lock(queue.mQueueMutex);
            return idx < queue.mQueue.size() ? queue.mQueue[idx] : nullptr;
        }
        static bool cancelled(const EntryPtr& entry){
            std::lock_guard<std::mutex> lock(entry->mutex);
            return entry->cancelled;
        }
    };
}

TEST(MainThreadQueueTests, timeoutCancelsWorkAlreadyTakenByPump){
    //The second caller's timeout races this thread reaching pump(), and a stalled thread loses: the
    //entry cancels and erases itself before pump can take it. Only an attempt where pump took it first
    //exercises the path, so retry until one does.
    bool exercised = false;
    for(int attempt = 0; attempt < 20 && !exercised; ++attempt){
        MainThreadQueue queue;
        std::atomic<bool> secondReturned{false};
        std::mutex releaseMutex;
        std::condition_variable releaseCv;
        MainThreadQueueTestAccess::EntryPtr secondEntry;
        bool takenBeforeCancel = false;
        bool mutationRan = false;
        auto first = std::async(std::launch::async, [&]{
            return queue.execute([&]{
                //Cancelling sets the flag and erases the entry under its mutex, so still uncancelled
                //here means it was still queued when pump swapped the queue out.
                takenBeforeCancel = secondEntry && !MainThreadQueueTestAccess::cancelled(secondEntry);
                std::unique_lock<std::mutex> lock(releaseMutex);
                releaseCv.wait_for(lock, std::chrono::seconds(1), [&]{ return secondReturned.load(); });
            }, 1000);
        });
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
        while(MainThreadQueueTestAccess::pending(queue) == 0 && std::chrono::steady_clock::now() < deadline) std::this_thread::yield();
        auto second = std::async(std::launch::async, [&]{
            bool result = queue.execute([&]{ mutationRan = true; }, 5);
            {
                std::lock_guard<std::mutex> lock(releaseMutex);
                secondReturned = true;
            }
            releaseCv.notify_one();
            return result;
        });
        while(MainThreadQueueTestAccess::pending(queue) < 2 && !secondReturned && std::chrono::steady_clock::now() < deadline) std::this_thread::yield();
        secondEntry = MainThreadQueueTestAccess::pendingAt(queue, 1);
        queue.pump();
        EXPECT_TRUE(first.get());
        EXPECT_FALSE(second.get());
        EXPECT_FALSE(mutationRan);
        exercised = takenBeforeCancel;
    }
    EXPECT_TRUE(exercised);
}

TEST(MainThreadQueueTests, executeRunsClosureExactlyOnceOnPump){
    MainThreadQueue queue;
    int count = 0;
    auto caller = std::async(std::launch::async, [&]{ return queue.execute([&]{ ++count; }, 1000); });
    EXPECT_TRUE(pumpUntilDone(queue, caller));
    queue.pump();
    EXPECT_EQ(count, 1);
}

TEST(MainThreadQueueTests, closureRunsOnPumpingThread){
    MainThreadQueue queue;
    const auto mainThread = std::this_thread::get_id();
    bool correctThread = false;
    auto caller = std::async(std::launch::async, [&]{
        return queue.execute([&]{ correctThread = std::this_thread::get_id() == mainThread; }, 1000);
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
        callers.emplace_back(std::async(std::launch::async, [&]{ return queue.execute([&]{ ++count; }, 1000); }));
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
