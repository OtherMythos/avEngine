#include "SequenceAnimationDef.h"
#include "AnimationInfoBlockUtil.h"
#include <cmath>
#include <cassert>
#include <iostream>

#include "AnimationManager.h"

#include "OgreSceneNode.h"
#include "OgreHlmsPbsDatablock.h"

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
        uint8 section = static_cast<uint8>(floor(float(anim.currentTime) / mStepCounter));
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
            for(uint32 k = definition.keyframeStart + definition.keyFrameSkip[section]; k <= definition.keyframeEnd; k++){
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
                if(keyframeEnd >= definition.keyframeEnd){
                    keyframeEnd = INVALID;
                    break;
                }
            }
            //One could be invalid (for instance right at the start of the timeline where there's no previous node).
            //However, they can't both be invalid, or the same.
            assert(keyframeStart != keyframeEnd);
            //If one of the keyframes is invalid then there's nothing to animate.
            if(keyframeStart == INVALID || keyframeEnd == INVALID) continue;
            assert(keyframeStart >= 0 && keyframeStart < mInfo.keyframes.size());
            assert(keyframeEnd >= 0 && keyframeEnd < mInfo.keyframes.size());

            //The two keyframes have been found.
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
            case AnimationTrackType::PBS_DIFFUSE: _processPbsDiffuseKeyframes(anim, track, k1, k2); break;
            case AnimationTrackType::PBS_DETAIL_MAP: _processPbsDetailMapKeyframes(anim, track, k1, k2); break;
            default: assert(false); break;
        }
    }

    Ogre::Vector3 _findVecDiff(bool position, float percentage, const AnimationDefConstructionInfo& info, const Keyframe& k1, const Keyframe& k2){
        uint32 start = position ? k1.a.ui : k1.b.ui;
        uint32 end = position ? k2.a.ui : k2.b.ui;
        Ogre::Vector3 startPos(info.data[start], info.data[start + 1], info.data[start + 2]);
        Ogre::Vector3 endPos(info.data[end], info.data[end + 1], info.data[end + 2]);
        Ogre::Vector3 diff(endPos - startPos);
        return startPos + diff*percentage;
    }

    bool _findDetailVec4(const AnimationDefConstructionInfo& info, const Keyframe& k, Ogre::Vector4& outVec){
        uint32 mask = k.data & 0x000000FF;
        bool setOffsetScale = false;
        if(mask & KeyframePbsDetailMapTypes::OffsetSet){
            outVec.x = info.data[k.a.ui];
            outVec.y = info.data[k.a.ui+1];
            setOffsetScale = true;
        }
        if(mask & KeyframePbsDetailMapTypes::ScaleSet){
            uint8 startId = setOffsetScale ? 2 : 0;
            outVec.z = info.data[k.a.ui+startId];
            outVec.w = info.data[k.a.ui+startId+1];
            setOffsetScale = true;
        }
        //Add checks to make sure the animations actually appear.
        return setOffsetScale;
    }

    bool _findDetailVec4Diff(float percentage, const AnimationDefConstructionInfo& info, const Keyframe& k1, const Keyframe& k2, Ogre::Vector4& outVec){
        Ogre::Vector4 targetOffsetScale1(outVec);
        Ogre::Vector4 targetOffsetScale2(outVec);
        bool usedFirst = _findDetailVec4(info, k1, targetOffsetScale1);
        bool usedSecond = _findDetailVec4(info, k2, targetOffsetScale2);

        Ogre::Vector4 diff(targetOffsetScale2 - targetOffsetScale1);
        outVec = targetOffsetScale1 + diff*percentage;

        return usedFirst && usedSecond;
    }

    void SequenceAnimationDef::_processPbsDiffuseKeyframes(SequenceAnimation& anim, const TrackDefinition& track, const Keyframe& k1, const Keyframe& k2){
        //TODO remove duplication.
        uint16 totalDistance = k2.keyframePos - k1.keyframePos;
        //Find the percentage of the way through.
        assert(anim.currentTime >= k1.keyframePos && anim.currentTime <= k2.keyframePos);
        float currentPercentage = float(anim.currentTime - k1.keyframePos) / float(totalDistance);

        AnimationInfoEntry animationEntry = _getInfoFromBlock(track.effectedData, anim.info.get());
        Ogre::HlmsPbsDatablock* targetBlock = animationEntry.pbsDatablock;
        const Ogre::Vector3 target = _findVecDiff(true, currentPercentage, mInfo, k1, k2);
        targetBlock->setDiffuse(target);
    }

    void SequenceAnimationDef::_processPbsDetailMapKeyframes(SequenceAnimation& anim, const TrackDefinition& track, const Keyframe& k1, const Keyframe& k2){
        //TODO remove duplication.
        uint16 totalDistance = k2.keyframePos - k1.keyframePos;
        //Find the percentage of the way through.
        assert(anim.currentTime >= k1.keyframePos && anim.currentTime <= k2.keyframePos);
        float currentPercentage = float(anim.currentTime - k1.keyframePos) / float(totalDistance);

        AnimationInfoEntry animationEntry = _getInfoFromBlock(track.effectedData, anim.info.get());
        Ogre::HlmsPbsDatablock* targetBlock = animationEntry.pbsDatablock;

        //TODO this should be set as part of the track.
        uint8 targetDetailMap = static_cast<uint8>(k1.data >> 8);

        Ogre::Vector4 targetOffsetScale(targetBlock->getDetailMapOffsetScale(targetDetailMap));
        bool valueFound = _findDetailVec4Diff(currentPercentage, mInfo, k1, k2, targetOffsetScale);
        if(valueFound){
            targetBlock->setDetailMapOffsetScale(targetDetailMap, targetOffsetScale);
        }

        uint32 mask = k1.data & 0x000000FF;
        if(mask & KeyframePbsDetailMapTypes::WeightSet){
            float diff = k2.b.f - k1.b.f;
            float target = k1.b.f + diff * currentPercentage;
            targetBlock->setDetailMapWeight(targetDetailMap, target);
        }
        if(mask & KeyframePbsDetailMapTypes::NormalWeightSet){
            float diff = k2.c.f - k1.c.f;
            float target = k1.c.f + diff * currentPercentage;
            targetBlock->setDetailNormalWeight(targetDetailMap, target);
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
            const Ogre::Vector3 target = _findVecDiff(true, currentPercentage, mInfo, k1, k2);
            targetNode->setPosition(target);
        }
        //TODO what if keyframe 2 does not specify any of these values.
        if(k1.data & KeyframeTransformTypes::Scale){
            const Ogre::Vector3 target = _findVecDiff(false, currentPercentage, mInfo, k1, k2);
            targetNode->setScale(target);
        }
        if(k1.data & KeyframeTransformTypes::Orientation){
            uint32 start = k1.c.ui;
            uint32 end = k2.c.ui;
            Ogre::Quaternion q1(mInfo.data[start], mInfo.data[start+1], mInfo.data[start+2], mInfo.data[start+3]);
            Ogre::Quaternion q2(mInfo.data[end], mInfo.data[end+1], mInfo.data[end+2], mInfo.data[end+3]);
            const Ogre::Quaternion target = Ogre::Quaternion::nlerp(currentPercentage, q1, q2);
            targetNode->setOrientation(target);
        }

    }

}
