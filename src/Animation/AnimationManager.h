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
        SequenceAnimationPtr createAnimation(SequenceAnimationDefPtr def, AnimationInfoBlockPtr info);
        AnimationInfoBlockPtr createAnimationInfoBlock(AnimationInfoEntry (&target)[MAX_ANIMATION_INFO], uint8 numAnimationInfo);
        void addAnimationDefinitionsFromParser(const AnimationParserOutput& info);

        SequenceAnimationDefPtr getAnimationDefinition(const std::string& animName);

        bool isAnimRunning(SequenceAnimationPtr p) const{
            return mAnimations.getEntryConst(p.get()).running;
        }
        uint16 getAnimTime(SequenceAnimationPtr p) const{
            return mAnimations.getEntryConst(p.get()).currentTime;
        }
        size_t getNumActiveAnimations() const{
            return mActiveAnimations.size();
        }

        void setAnimationRunning(SequenceAnimationPtr p, bool running);

        /**
        Called as part of animation def destruction.
        */
        void _removeCreatedAnimationDef(const std::string& animName);

    private:
        std::map<std::string, std::shared_ptr<SequenceAnimationDef>> mAnimationDefs;
        DataPacker<SequenceAnimation> mAnimations;
        std::set<void*> mActiveAnimations;
        //Previously running animations now queued for removal.
        std::set<void*> mQueuedEndAnimations;

        static void _destroyAnimationInstance(void* object);
        static void _destroyAnimationInfoBlockInstance(void* object);
    };
}
