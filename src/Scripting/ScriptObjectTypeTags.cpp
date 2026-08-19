#include "ScriptObjectTypeTags.h"

#include <cstdlib>
#include <limits>

namespace{
    //Engine tags currently occupy values below 500. This counter deliberately lives in an
    //engine source file so dynamic and static plugins all allocate from the same instance.
    std::atomic<std::uintptr_t> nextCustomTypeTag(501);

    std::uintptr_t claimCustomTypeTag() noexcept{
        std::uintptr_t candidate = nextCustomTypeTag.load(std::memory_order_relaxed);
        const std::uintptr_t maximum = std::numeric_limits<std::uintptr_t>::max();

        while(candidate != maximum){
            if(nextCustomTypeTag.compare_exchange_weak(
                candidate,
                candidate + 1,
                std::memory_order_relaxed,
                std::memory_order_relaxed
            )){
                return candidate;
            }
        }

        //Exhausting the address-sized tag space is not recoverable: returning an existing tag
        //would silently make two unrelated Squirrel types compare equal.
        std::abort();
    }
}

namespace AV{
    void* ScriptObjectTypeTag::get() const noexcept{
        std::uintptr_t value = mValue.load(std::memory_order_acquire);
        if(value == 0){
            const std::uintptr_t claimed = claimCustomTypeTag();
            if(mValue.compare_exchange_strong(
                value,
                claimed,
                std::memory_order_release,
                std::memory_order_acquire
            )){
                value = claimed;
            }
            //If another thread won the race, compare_exchange_strong placed that thread's
            //stored value in value. The unused claimed integer is intentionally never reused.
        }

        return reinterpret_cast<void*>(value);
    }
}
