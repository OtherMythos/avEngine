#include "SequenceAnimationDef.h"
#include <cmath>
#include <cassert>
#include <iostream>

namespace AV{
    SequenceAnimationDef::SequenceAnimationDef(const AnimationDefConstructionInfo& info)
    //Some temporary information.
        : mRepeats(false),
          mLength(40),
          mStepCounter(mLength / 4.0f) {

        mTrackDefinition = {
            //TODO does the start count as section 0 on its own.
            {AnimationTrackType::DatablockAnimate, 0, {0, 2, 2, 2}}
        };
        mKeyframes = {
            {5, 0, 0},
            {9, 0, 0}
        };
    }

    SequenceAnimationDef::~SequenceAnimationDef(){

    }

    bool SequenceAnimationDef::update(SequenceAnimation& anim){
        assert(anim.currentTime <= mLength);
        uint8 section = floor(float(anim.currentTime) / mStepCounter);
        //TODO I'd like to avoid this if.
        if(section == 4) section = 3;
        assert(section < 4);
        for(const TrackDefinition& definition : mTrackDefinition){
            //TODO this could be one variable with just a -1.
            uint32 keyframeStart, keyframeEnd;
            static const uint32 INVALID = 0xFFFFFFFF;
            keyframeStart = keyframeEnd = INVALID;
            for(uint32 k = definition.keyframeStart + definition.keyFrameSkip[section]; k <= definition.keyframeStart + definition.keyFrameSkip[3]; k++){
                //Find the start and end value
                if(mKeyframes[k].keyframePos < anim.currentTime) continue;
                assert(mKeyframes[k].keyframePos >= anim.currentTime);

                //The current checking one is greater, which means the cursor is between two points.
                if(mKeyframes[k].keyframePos > anim.currentTime){
                    keyframeEnd = k;
                    keyframeStart = k - 1;
                    break;
                }
                //If not, the cursor has landed on the new keyframe.
                keyframeStart = k;
                keyframeEnd = k + 1;
            }
            //One could be invalid (for instance right at the start of the timeline where there's no previous node).
            //However, they can't both be invalid, or the same.
            assert(keyframeStart != keyframeEnd);

            //The two keyframes have been found.
            std::cout << keyframeStart << std::endl;
        }

        anim.currentTime++;
        if(anim.currentTime > mLength) {
            if(mRepeats) anim.currentTime = 0;
            //Notify that the animation is no longer running.
            else return false;
        }
        return true;
    }

    void SequenceAnimationDef::progressAnimationWithKeyframes(SequenceAnimation& anim, uint32 k1, uint32 k2){
        //Update depending on the type of animation.
    }

}
