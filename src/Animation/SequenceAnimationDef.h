#pragma once

#include "AnimationData.h"

#include <vector>

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

    private:
        struct TrackDefinition{
            AnimationTrackType type;
            uint32 keyframeStart;
            uint8 keyFrameSkip[4]; //keyframeSkip1, keyframeSkip2, keyframeSkip3, keyframeEnd;
        };
        union KeyFrameData{
            int i;
            bool b;
            float f;
        };
        struct Keyframe{
            uint16 keyframePos;
            KeyFrameData a;
            KeyFrameData b;
        };

        std::vector<TrackDefinition> mTrackDefinition;
        std::vector<Keyframe> mKeyframes;
        bool mRepeats;
        uint16 mLength;
        float mStepCounter;

        void progressAnimationWithKeyframes(SequenceAnimation& anim, uint32 k1, uint32 k2);
    };
}
