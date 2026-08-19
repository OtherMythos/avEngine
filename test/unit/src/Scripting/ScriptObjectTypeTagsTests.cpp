#include "gtest/gtest.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include <atomic>
#include <thread>

TEST(ScriptObjectTypeTagsTests, returnsStableNonNullTag){
    const AV::ScriptObjectTypeTag tag;

    void* first = tag.get();

    ASSERT_NE(first, nullptr);
    ASSERT_EQ(tag.get(), first);
}

TEST(ScriptObjectTypeTagsTests, separateObjectsReceiveDifferentTags){
    const AV::ScriptObjectTypeTag first;
    const AV::ScriptObjectTypeTag second;

    ASSERT_NE(first.get(), second.get());
}

TEST(ScriptObjectTypeTagsTests, concurrentFirstAccessReturnsOneTag){
    const AV::ScriptObjectTypeTag tag;
    std::atomic<int> ready(0);
    std::atomic<bool> run(false);
    void* results[2] = {nullptr, nullptr};

    std::thread first([&]{
        ready.fetch_add(1, std::memory_order_release);
        while(!run.load(std::memory_order_acquire)) std::this_thread::yield();
        results[0] = tag.get();
    });
    std::thread second([&]{
        ready.fetch_add(1, std::memory_order_release);
        while(!run.load(std::memory_order_acquire)) std::this_thread::yield();
        results[1] = tag.get();
    });

    while(ready.load(std::memory_order_acquire) != 2) std::this_thread::yield();
    run.store(true, std::memory_order_release);
    first.join();
    second.join();

    ASSERT_NE(results[0], nullptr);
    ASSERT_EQ(results[0], results[1]);
    ASSERT_EQ(results[0], tag.get());
}
