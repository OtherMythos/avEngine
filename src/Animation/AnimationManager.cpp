#include "AnimationManager.h"

#include "SequenceAnimationDef.h"
#include "System/BaseSingleton.h"

namespace AV{
    AnimationManager::AnimationManager(){

    }

    AnimationManager::~AnimationManager(){

    }

    void AnimationManager::update(){
        for(void* id : mActiveAnimations){
            SequenceAnimation& anim = mAnimations.getEntry(id);
            //Should be running if it's in this list.
            assert(anim.running);
            anim.def->update(anim);
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

        entry.def.reset();
        animManager->mAnimations.removeEntry(object);
    }
}
