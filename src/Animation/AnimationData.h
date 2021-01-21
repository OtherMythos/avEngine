#pragma once

#include "System/EnginePrerequisites.h"
#include <memory>
#include <vector>

namespace Ogre{
    class SceneNode;
}

namespace AV{
    class SequenceAnimationDef;

    typedef std::shared_ptr<SequenceAnimationDef> SequenceAnimationDefPtr;
    typedef std::shared_ptr<void> SequenceAnimationPtr;
    typedef std::shared_ptr<void> AnimationInfoBlockPtr;

    enum class AnimationTrackType{
        Transform,
        DatablockAnimate
    };
    enum class AnimationKeyframeType{
        //Transform
        Position,
        Scale,
        Orientation

        //Datablock
    };

    struct TrackDefinition{
        AnimationTrackType type;
        uint32 keyframeStart;
        uint8 keyFrameSkip[4]; //keyframeSkip1, keyframeSkip2, keyframeSkip3, keyframeEnd;
        uint8 effectedData;
    };
    union KeyFrameData{
        int i;
        uint ui;
        bool b;
        float f;
    };
    struct Keyframe{
        uint16 keyframePos;
        //Generit pieces of data which can be used to define what the keyframe contains or does.
        uint32 data;
        KeyFrameData a;
        KeyFrameData b;
        KeyFrameData c;
    };

    struct AnimationDefConstructionInfo{
        AnimationDefConstructionInfo(
            bool r, uint16 l,
            const std::vector<TrackDefinition>& td,
            const std::vector<Keyframe>& k,
            const std::vector<float>& d
        )
        : repeats(r), length(l),
        trackDefinition(td), keyframes(k), data(d) { }
        AnimationDefConstructionInfo() { };

        bool repeats;
        uint16 length;
        std::vector<TrackDefinition> trackDefinition;
        std::vector<Keyframe> keyframes;
        std::vector<float> data;
    };

    struct SequenceAnimation{
        SequenceAnimationDefPtr def;
        AnimationInfoBlockPtr info;
        uint16 currentTime;
        bool running;
    };

    static const uint8 MAX_ANIMATION_INFO = 16;
    union AnimationInfoEntry{
        Ogre::SceneNode* sceneNode;
    };
}