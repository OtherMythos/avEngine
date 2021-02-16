#pragma once

#include <cassert>
#include "AnimationData.h"

namespace AV{
    static AnimationInfoEntry _getInfoFromBlock(uint8 idx, void* ptr){
        uint8* value = reinterpret_cast<uint8*>(ptr);
        assert(*value < MAX_ANIMATION_INFO);
        value++;
        uint64* value2 = reinterpret_cast<uint64*>(value);
        value2++;
        AnimationInfoEntry* entry = reinterpret_cast<AnimationInfoEntry*>(value2);
        entry += idx;
        return *entry;
    }

    static AnimationInfoTypeHash _getHashFromBlock(void* ptr){
        uint8* value = reinterpret_cast<uint8*>(ptr);
        value++;
        uint64* value2 = reinterpret_cast<uint64*>(value);
        return *value2;
    }

    static AnimationInfoTypeHash _produceTypeHashForObjectTypes(AnimationInfoTypes (&types)[MAX_ANIMATION_INFO]){
        AnimationInfoTypeHash out = 0;
        for(uint8 i = 0; i < MAX_ANIMATION_INFO; i++){
            assert(types[i] < ANIM_INFO_MAX);
            out |= static_cast<AnimationInfoTypeHash>(types[i]) << i*4;
        }

        return out;
    }
}
