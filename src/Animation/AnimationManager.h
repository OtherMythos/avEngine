#pragma once

#include <string>
#include <map>
#include <set>

#include "AnimationData.h"
#include "System/Util/DataPacker.h"

namespace AV{

    /**
    Manages sequence animations.
    This includes their lifetime, creation and the means to update them.
    */
    class AnimationManager{
    public:
        AnimationManager();
        ~AnimationManager();

        void update();

        SequenceAnimationDefPtr createAnimationDefinition(const std::string& animName, const AnimationDefConstructionInfo& info);
        SequenceAnimationPtr createAnimation(SequenceAnimationDefPtr def);

    private:
        std::map<std::string, std::weak_ptr<SequenceAnimationDef>> mAnimationDefs;
        DataPacker<SequenceAnimation> mAnimations;
        std::set<void*> mActiveAnimations;

        static void _destroyAnimationInstance(void* object);
    };
}
