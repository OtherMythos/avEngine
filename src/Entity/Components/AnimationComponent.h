#pragma once

#include "Animation/AnimationData.h"

namespace AV{
    struct AnimationComponent{
        AnimationComponent(SequenceAnimationPtr a, SequenceAnimationPtr b, bool aPop, bool bPop)
            : aPopulated(aPop), bPopulated(bPop), objA(a), objB(b) { }
        ~AnimationComponent(){
            objA.reset();
            objB.reset();
        }

        bool aPopulated;
        bool bPopulated;
        SequenceAnimationPtr objA;
        SequenceAnimationPtr objB;

    };
}
