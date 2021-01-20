#pragma once

#include <cassert>
#include "AnimationData.h"

namespace AV{
    AnimationInfoEntry _getInfoFromBlock(uint8 idx, void* ptr){
        uint8* value = reinterpret_cast<uint8*>(ptr);
        assert(*value < MAX_ANIMATION_INFO);
        value++;
        AnimationInfoEntry* entry = reinterpret_cast<AnimationInfoEntry*>(value);
        entry += idx;
        return *entry;
    }
}
