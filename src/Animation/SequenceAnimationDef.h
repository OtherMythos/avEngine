#pragma once

#include "AnimationData.h"

namespace AV{

    /**
    Definition of a sequence animation.
    This class contains all the relevant information for a single animation sequence,
    and is intended to be shared among many animations.
    Individual animation objects just reference the animation definition and information relevant to them such as current time.
    */
    class SequenceAnimationDef{
    public:
        SequenceAnimationDef(const AnimationDefConstructionInfo& info);
        ~SequenceAnimationDef();

        /**
        Update an animation with this specific animation definition.
        @returns True or false depending on whether the animation finished this frame.
        */
        bool update(SequenceAnimation& anim);
    };
}
