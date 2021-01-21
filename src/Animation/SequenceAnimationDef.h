#pragma once

#include "AnimationData.h"

#include <vector>

namespace AV{
    namespace KeyframeTransformTypes{
        enum KeyframeTransformTypes{
            Position = 1u << 0,
            Scale = 1u << 1,
            Orientation = 1u << 2
        };
    }

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

    private:

        // std::vector<TrackDefinition> mTrackDefinition;
        // std::vector<Keyframe> mKeyframes;
        // bool mRepeats;
        // uint16 mLength;
        AnimationDefConstructionInfo mInfo;
        float mStepCounter;

        void progressAnimationWithKeyframes(SequenceAnimation& anim, const TrackDefinition& track, const Keyframe& k1, const Keyframe& k2);
        void _processTransformKeyframes(SequenceAnimation& anim, const TrackDefinition& track, const Keyframe& k1, const Keyframe& k2);

        void _populateFromDef(const AnimationDefConstructionInfo& info);
    };
}
