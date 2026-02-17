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

        void shutdown();

        void update();

        //TODO this function isn't used anywhere.
        SequenceAnimationDefPtr createAnimationDefinition(const std::string& animName, const AnimationDefConstructionInfo& info);
        /**
        Create an animation from an animation def.
        See getAnimationDefinition.
        The provided data info block must match the required types. See createAnimationInfoBlock.
        */
        SequenceAnimationPtr createAnimation(SequenceAnimationDefPtr def, AnimationInfoBlockPtr info, bool& success);
        /**
        Create a block of animation data. The returned object will be destroyed when all references reach 0.
        @param target: A collection of object infos to be written to the created object.
        @param numAnimationInfo: The number of objects in the provided target array which are valid.
        @param hash: The type hash of the outputted object. See AnimationInfoBlockUtil::_produceTypeHashForObjectTypes()
        */
        AnimationInfoBlockPtr createAnimationInfoBlock(AnimationInfoEntry (&target)[MAX_ANIMATION_INFO], uint8 numAnimationInfo, AnimationInfoTypeHash hash);
        /**
        Add the animations derived from a parsed file into the manager.
        If an animation with that name already exists it will be ignored.
        */
        void addAnimationDefinitionsFromParser(const AnimationParserOutput& info);

        /**
        Obtain an animation definition by name.
        @returns A valid animation def if one with that name is found. 0 if not.
        */
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

        /**
        Set an animation to the 'running' state.
        When running and animation is updated automatically each frame.
        */
        void setAnimationRunning(SequenceAnimationPtr p, bool running);

        /**
        Set the time of the animation.
        @param update: Whether to force an update to this animation after the time has been set.
        */
        void setAnimTime(SequenceAnimationPtr p, uint16 time, bool update=false);

        /**
        Set the pause mask for a specific animation.
        */
        void setAnimationPauseMask(SequenceAnimationPtr p, uint32 mask);

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
        bool mShuttingDown;

        static void _destroyAnimationInstance(void* object);
        static void _destroyAnimationInfoBlockInstance(void* object);

        void _updateActiveAnimations();
    };
}
