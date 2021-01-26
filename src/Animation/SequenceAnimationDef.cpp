#include "SequenceAnimationDef.h"
#include "AnimationInfoBlockUtil.h"
#include <cmath>
#include <cassert>
#include <iostream>

#include "AnimationManager.h"

#include "OgreSceneNode.h"

namespace AV{
    SequenceAnimationDef::SequenceAnimationDef(const AnimationDefConstructionInfo& info, const std::string& name, AnimationManager* creator)
        : mInfo(info), mStepCounter(info.length / 4), animManager(creator), animName(name) {
    }

    SequenceAnimationDef::SequenceAnimationDef(size_t idx, const AnimationParserOutput& info, const std::string& name, AnimationManager* creator)
        : animManager(creator), animName(name) {
        assert(idx < info.animInfo.size());
        const AnimationParserOutput::AnimationInfo& animInfo = info.animInfo[idx];
        mInfo.length = animInfo.length;
        mInfo.repeats = animInfo.repeats;
        mInfo.animInfoHash = info.infoHashes[animInfo.targetAnimInfoHash];

        mInfo.data.clear();
        mInfo.keyframes.clear();
        mInfo.trackDefinition.clear();

        mInfo.data.insert(mInfo.data.begin(), info.data.begin()+animInfo.dataStart, info.data.begin()+animInfo.dataEnd);
        mInfo.keyframes.insert(mInfo.keyframes.begin(), info.keyframes.begin()+animInfo.keyframeStart, info.keyframes.begin()+animInfo.keyframeEnd);
        mInfo.trackDefinition.insert(mInfo.trackDefinition.begin(), info.trackDefinition.begin()+animInfo.trackStart, info.trackDefinition.begin()+animInfo.trackEnd);
    }

    SequenceAnimationDef::~SequenceAnimationDef(){
        animManager->_removeCreatedAnimationDef(animName);
    }

    bool SequenceAnimationDef::update(SequenceAnimation& anim){
        assert(anim.currentTime <= mInfo.length);
        uint8 section = floor(float(anim.currentTime) / mStepCounter);
        //TODO I'd like to avoid this if.
        if(section == 4) section = 3;
        assert(section < 4);
        for(const TrackDefinition& definition : mInfo.trackDefinition){
            //A track with no keyframes should not make it into the final list.
            assert(definition.keyframeStart != definition.keyframeEnd);
            //TODO this could be one variable with just a -1.
            uint32 keyframeStart, keyframeEnd;
            static const uint32 INVALID = 0xFFFFFFFF;
            keyframeStart = keyframeEnd = INVALID;
            for(uint32 k = definition.keyframeStart + definition.keyFrameSkip[section]; k <= definition.keyframeStart + definition.keyFrameSkip[3]; k++){
                //Find the start and end value
                if(mInfo.keyframes[k].keyframePos < anim.currentTime) continue;
                assert(mInfo.keyframes[k].keyframePos >= anim.currentTime);

                //The current checking one is greater, which means the cursor is between two points.
                if(mInfo.keyframes[k].keyframePos > anim.currentTime){
                    keyframeEnd = k;
                    keyframeStart = k - 1;
                    break;
                }
                //If not, the cursor has landed on the new keyframe.
                keyframeStart = k;
                keyframeEnd = k + 1;
                if(keyframeEnd >= definition.keyframeEnd) keyframeEnd = INVALID;
            }
            //One could be invalid (for instance right at the start of the timeline where there's no previous node).
            //However, they can't both be invalid, or the same.
            assert(keyframeStart != keyframeEnd);
            //If one of the keyframes is invalid then there's nothing to animate.
            if(keyframeStart == INVALID || keyframeEnd == INVALID) continue;

            //The two keyframes have been found.
            std::cout << keyframeStart << " found value " << keyframeEnd << " " << mInfo.keyframes.size() << std::endl;
            progressAnimationWithKeyframes(anim, definition, mInfo.keyframes[keyframeStart], mInfo.keyframes[keyframeEnd]);
        }

        anim.currentTime++;
        if(anim.currentTime > mInfo.length){
            if(mInfo.repeats) anim.currentTime = 0;
            //Notify that the animation is no longer running.
            else return false;
        }
        return true;
    }

    void SequenceAnimationDef::progressAnimationWithKeyframes(SequenceAnimation& anim, const TrackDefinition& track, const Keyframe& k1, const Keyframe& k2){
        //Update depending on the type of animation.
        switch(track.type){
            case AnimationTrackType::Transform: _processTransformKeyframes(anim, track, k1, k2); break;
            default: assert(false); break;
        }
    }

    void SequenceAnimationDef::_processTransformKeyframes(SequenceAnimation& anim, const TrackDefinition& track, const Keyframe& k1, const Keyframe& k2){
        uint16 totalDistance = k2.keyframePos - k1.keyframePos;
        //Find the percentage of the way through.
        assert(anim.currentTime >= k1.keyframePos && anim.currentTime <= k2.keyframePos);
        float currentPercentage = float(anim.currentTime - k1.keyframePos) / float(totalDistance);

        AnimationInfoEntry animationEntry = _getInfoFromBlock(track.effectedData, anim.info.get());
        Ogre::SceneNode* targetNode = animationEntry.sceneNode;
        if(k1.data & KeyframeTransformTypes::Position){
            std::cout << k1.a.ui << "  " << k2.a.ui << std::endl;
            std::cout << targetNode->getPosition() << std::endl;

            Ogre::Vector3 startPos(mInfo.data[k1.a.ui], mInfo.data[k1.a.ui + 1], mInfo.data[k1.a.ui + 2]);
            Ogre::Vector3 endPos(mInfo.data[k2.a.ui], mInfo.data[k2.a.ui + 1], mInfo.data[k2.a.ui + 2]);
            Ogre::Vector3 diff(endPos - startPos);
            std::cout << "diff " << diff << std::endl;
            Ogre::Vector3 target(startPos + diff*currentPercentage);

            std::cout << startPos << "  " << endPos << std::endl;
            std::cout << currentPercentage << std::endl;
            targetNode->setPosition(target);
        }

    }

}
