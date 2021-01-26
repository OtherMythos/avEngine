#pragma once

#include "System/EnginePrerequisites.h"
#include <memory>
#include <vector>
#include <string>

namespace Ogre{
    class SceneNode;
}

namespace AV{
    class SequenceAnimationDef;

    typedef std::shared_ptr<SequenceAnimationDef> SequenceAnimationDefPtr;
    typedef std::shared_ptr<void> SequenceAnimationPtr;
    typedef std::shared_ptr<void> AnimationInfoBlockPtr;

    enum class AnimationTrackType{
        None,
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
        uint32 keyframeStart, keyframeEnd;
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
        //Generic pieces of data which can be used to define what the keyframe contains or does.
        uint32 data;
        KeyFrameData a;
        KeyFrameData b;
        KeyFrameData c;
    };

    namespace KeyframeTransformTypes{
        enum KeyframeTransformTypes{
            Position = 1u << 0,
            Scale = 1u << 1,
            Orientation = 1u << 2
        };
    }

    static const uint8 MAX_ANIMATION_INFO = 16;
    union AnimationInfoEntry{
        Ogre::SceneNode* sceneNode;
    };
    enum AnimationInfoTypes : uint8{
        ANIM_INFO_NONE = 0,
        ANIM_INFO_SCENE_NODE,
        ANIM_INFO_MAX
    };
    typedef uint64 AnimationInfoTypeHash;

    struct AnimationDefConstructionInfo{
        AnimationDefConstructionInfo(
            bool r, uint16 l,
            const std::vector<TrackDefinition>& td,
            const std::vector<Keyframe>& k,
            const std::vector<float>& d
        )
        : repeats(r), length(l),
        trackDefinition(td), keyframes(k), data(d), animInfoHash(0) { }
        AnimationDefConstructionInfo() { };

        bool repeats;
        uint16 length;
        std::vector<TrackDefinition> trackDefinition;
        std::vector<Keyframe> keyframes;
        std::vector<float> data;
        AnimationInfoTypeHash animInfoHash;
    };

    struct SequenceAnimation{
        SequenceAnimationDefPtr def;
        AnimationInfoBlockPtr info;
        uint16 currentTime;
        bool running;
    };

    //Animation info output populated by the AnimationScriptParser.
    //Multiple animations can be stored in a single instance of this object.
    struct AnimationParserOutput{
        struct AnimationInfo{
            std::string name;
            bool repeats;
            uint16 length;
            uint8 targetAnimInfoHash;

            size_t trackStart, trackEnd;
            size_t keyframeStart, keyframeEnd;
            size_t dataStart, dataEnd;
        };
        std::vector<TrackDefinition> trackDefinition;
        std::vector<Keyframe> keyframes;
        std::vector<float> data;
        std::vector<AnimationInfo> animInfo;
        std::vector<AnimationInfoTypeHash> infoHashes;
    };
}
