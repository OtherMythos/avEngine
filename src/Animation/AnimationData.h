#pragma once

#include "System/EnginePrerequisites.h"
#include <memory>

namespace Ogre{
    class SceneNode;
}

namespace AV{
    class SequenceAnimationDef;

    typedef std::shared_ptr<SequenceAnimationDef> SequenceAnimationDefPtr;
    typedef std::shared_ptr<void> SequenceAnimationPtr;
    typedef std::shared_ptr<void> AnimationInfoBlockPtr;

    struct AnimationDefConstructionInfo{
        //Some temporary value.
        int value;
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
}
