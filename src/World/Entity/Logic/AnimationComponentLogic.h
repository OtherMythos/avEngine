#pragma once

#include "ComponentLogic.h"
#include "Animation/AnimationData.h"

namespace AV{
    class SerialiserStringStore;

    class AnimationComponentLogic : public ComponentLogic{
    public:
        static bool add(eId id, SequenceAnimationPtr a, SequenceAnimationPtr b, bool aPopulated, bool bPopulated);
        static bool remove(eId id);

        static SequenceAnimationPtr getAnimation(eId id, uint8 animId);
    };
}
