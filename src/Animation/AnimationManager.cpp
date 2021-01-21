#include "AnimationManager.h"

#include "SequenceAnimationDef.h"
#include "System/BaseSingleton.h"

#include <cstring>

//TODO temporary
#include "World/Support/OgreMeshManager.h"

namespace AV{
    AnimationManager::AnimationManager(){
    }

    AnimationManager::~AnimationManager(){

    }

    void AnimationManager::createTestAnimation(){
        AnimationDefConstructionInfo info;
        info.repeats = false;
        info.length = 80;
        info.trackDefinition = {
            {AnimationTrackType::Transform, 0, {0, 1, 2, 2}, 0}
        };
        KeyFrameData d;
        d.i = 1;
        info.keyframes = {
            {0, KeyframeTransformTypes::Position, 0, 0, 0},
            {40, KeyframeTransformTypes::Position, 1, 0, 0},
            {80, KeyframeTransformTypes::Position, 2, 0, 0}
        };
        info.data = {
            0, 0, 0,
            50, 30, 0,
            100, 0, 0
        };
        SequenceAnimationDefPtr animDef = createAnimationDefinition("testAnimation", info);

        AnimationInfoEntry values[MAX_ANIMATION_INFO];
        memset(&values, 0, sizeof(values));
        values[0].sceneNode = BaseSingleton::getOgreMeshManager()->mParentEntityNode;
        AnimationInfoBlockPtr infoPtr = createAnimationInfoBlock(values, 1);

        static SequenceAnimationPtr sequenceAnim = createAnimation(animDef, infoPtr);
    }

    void AnimationManager::update(){
        static bool hasTestAnim = false;
        if(!hasTestAnim) { createTestAnimation(); hasTestAnim = true; }
        for(void* id : mActiveAnimations){
            SequenceAnimation& anim = mAnimations.getEntry(id);
            //Should be running if it's in this list.
            assert(anim.running);
            bool stillRunning = anim.def->update(anim);
            if(!stillRunning){
                //Queue the removal for later so it doesn't interfere with the iteration.
                mQueuedEndAnimations.insert(id);
            }
        }

        if(!mQueuedEndAnimations.empty()){
            for(void* id : mQueuedEndAnimations){
                mActiveAnimations.erase(id);
            }
            mQueuedEndAnimations.clear();
        }
    }

    SequenceAnimationDefPtr AnimationManager::createAnimationDefinition(const std::string& animName, const AnimationDefConstructionInfo& info){
        auto it = mAnimationDefs.find(animName);
        if(it != mAnimationDefs.end()){
            //An animation with that name already exists in the manager.
            if(!it->second.expired()) return 0;
        }
        //Otherwise make a new one.
        SequenceAnimationDefPtr p = std::make_shared<SequenceAnimationDef>(info);
        mAnimationDefs[animName] = p;
        return p;
    }

    SequenceAnimationPtr AnimationManager::createAnimation(SequenceAnimationDefPtr def, AnimationInfoBlockPtr info){
        void* storedEntry = mAnimations.storeEntry({def, info, 0, true});
        mActiveAnimations.insert(storedEntry);

        SequenceAnimationPtr sharedPtr = SequenceAnimationPtr(storedEntry, _destroyAnimationInstance);
        return sharedPtr;
    }

    AnimationInfoBlockPtr AnimationManager::createAnimationInfoBlock(AnimationInfoEntry (&targetBlocks)[MAX_ANIMATION_INFO], uint8 numAnimationInfo){
        //Right now I'm storing these on the heap for convenience. In future a custom memory management solution is possible.
        //I have to calculate how much size I need for each object, based on the intended contents.
        //I chose the malloc approach because it means I can vary the size of the object in memory based on how many items it needs.
        //I would otherwise need to have MAX_ANIMATION_INFO objects each time.
        size_t requiredSize = sizeof(AnimationInfoEntry) * numAnimationInfo;
        requiredSize += sizeof(uint8) * 1;
        void* target = malloc(requiredSize);
        assert(malloc);

        { //Populate the values in the new data.
            uint8* numValues = reinterpret_cast<uint8*>(target);
            *numValues = numAnimationInfo;
            numValues++;
            AnimationInfoEntry* entryPtr = reinterpret_cast<AnimationInfoEntry*>(numValues);
            for(uint8 i = 0; i < numAnimationInfo; i++){
                *entryPtr++ = targetBlocks[i];
            }
        }

        AnimationInfoBlockPtr targetPtr(target, _destroyAnimationInfoBlockInstance);
        return targetPtr;
    }

    void AnimationManager::_destroyAnimationInfoBlockInstance(void* object){
        free(object);
    }

    void AnimationManager::_destroyAnimationInstance(void* object){
        auto animManager = BaseSingleton::getAnimationManager();
        SequenceAnimation& entry = animManager->mAnimations.getEntry(object);
        //Make sure there's no chance this can be called when updating an animation (for instance if I can later call scripts from an animation).
        animManager->mActiveAnimations.erase(object);

        entry.def.reset();
        animManager->mAnimations.removeEntry(object);
    }
}
