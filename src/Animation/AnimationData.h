#pragma once

#include "System/EnginePrerequisites.h"
#include <memory>

namespace AV{
    class SequenceAnimationDef;

    typedef std::shared_ptr<SequenceAnimationDef> SequenceAnimationDefPtr;
    typedef std::shared_ptr<void> SequenceAnimationPtr;

    struct AnimationDefConstructionInfo{
        //Some temporary value.
        int value;
    };

    struct SequenceAnimation{
        SequenceAnimationDefPtr def;
        uint16 currentTime;
        bool running;
    };
}
