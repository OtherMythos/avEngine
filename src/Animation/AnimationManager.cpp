#include "AnimationManager.h"

#include "SequenceAnimationDef.h"
#include "System/BaseSingleton.h"

namespace AV{
    AnimationManager::AnimationManager(){
        createTestAnimation();
    }

    AnimationManager::~AnimationManager(){

    }

    void AnimationManager::createTestAnimation(){
        SequenceAnimationDefPtr animDef = createAnimationDefinition("testAnimation", AnimationDefConstructionInfo());

        static SequenceAnimationPtr sequenceAnim = createAnimation(animDef);
    }

    void AnimationManager::update(){
        for(void* id : mActiveAnimations){
            SequenceAnimation& anim = mAnimations.getEntry(id);
            //Should be running if it's in this list.
            assert(anim.running);
            bool stillRunning = anim.def->update(anim);
            if(!stillRunning){
                mActiveAnimations.erase(id);
            }
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

    SequenceAnimationPtr AnimationManager::createAnimation(SequenceAnimationDefPtr def){
        void* storedEntry = mAnimations.storeEntry({def, 0, true});
        mActiveAnimations.insert(storedEntry);

        SequenceAnimationPtr sharedPtr = SequenceAnimationPtr(storedEntry, _destroyAnimationInstance);
        return sharedPtr;
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
