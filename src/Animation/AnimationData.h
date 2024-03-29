#pragma once

#include "System/EnginePrerequisites.h"
#include <memory>
#include <vector>
#include <string>

namespace Ogre{
    class SceneNode;
    class HlmsPbsDatablock;
}

namespace AV{
    class SequenceAnimationDef;

    typedef std::shared_ptr<SequenceAnimationDef> SequenceAnimationDefPtr;
    typedef std::shared_ptr<void> SequenceAnimationPtr;
    typedef std::shared_ptr<void> AnimationInfoBlockPtr;

    static const AV::uint8 ANIM_EASING_SHIFT_BITS = 24;
    //The skiplist describes the four quarters, but I only store three values as the first always starts at keyframe 0.
    static const AV::uint8 NUM_KEYFRAME_SKIP = 3;

    enum class AnimationTrackType{
        None,
        Transform,
        PBS_DIFFUSE,
        PBS_DETAIL_MAP,
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
        //Start and end indexes into the list. Counts from 0 per animation.
        uint32 keyframeStart, keyframeEnd;
        uint8 keyFrameSkip[NUM_KEYFRAME_SKIP]; //keyframeSkip1, keyframeSkip2, keyframeSkip3;
        uint8 effectedData;
        //Data used depending on which track type this is.
        uint32 userData;
    };
    union KeyFrameData{
        int i;
        uint32 ui;
        bool b;
        float f;
    };
    struct Keyframe{
        //The position in the timeline where the keyframe exists.
        uint16 keyframePos;
        //Generic pieces of data which can be used to define what the keyframe contains or does.
        uint32 data;
        //Generic pieces of data. These will be interpreted depending on the track type.
        KeyFrameData a;
        KeyFrameData b;
        KeyFrameData c;
    };

    namespace KeyframeTransformTypes{
        enum KeyframeTransformTypes : uint32{
            Position = 1u << 0,
            Scale = 1u << 1,
            Orientation = 1u << 2,
            Rotation = 1u << 3,
        };
    }

    namespace KeyframePbsSetTypes{
        enum KeyframePbsSetTypes : uint32{
            //TODO is this necessary
            DiffuseSet = 1u << 0,
        };
    }

    namespace KeyframePbsDetailMapTypes{
        enum KeyframePbsDetailMapTypes : uint32{
            OffsetSet = 1u << 0,
            ScaleSet = 1u << 1,
            WeightSet = 1u << 2,
            NormalWeightSet = 1u << 3,
        };
    }

    namespace FrameEasingType{
        enum FrameEasingType : uint32{
            //Start at 0 as linear is the default.
            FrameEasingLinear = 0,
            FrameEasingEaseInSine,
            FrameEasingEaseOutSine,
            FrameEasingEaseInOutSine,
            FrameEasingEaseInCubic,
            FrameEasingEaseOutCubic,
            FrameEasingEaseInOutCubic,
        };
    }

    static const uint8 MAX_ANIMATION_INFO = 16;
    static const uint8 MAX_ANIMATION_INFO_BITS = 64/MAX_ANIMATION_INFO;
    union AnimationInfoEntry{
        Ogre::SceneNode* sceneNode;
        Ogre::HlmsPbsDatablock* pbsDatablock;
    };
    enum AnimationInfoTypes : uint8{
        ANIM_INFO_NONE = 0,
        ANIM_INFO_SCENE_NODE,
        ANIM_INFO_PBS_DATABLOCK,
        ANIM_INFO_MAX
    };
    //Four bits are used for the current max of 16 infos per data. This means a max of 16 different values.
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
