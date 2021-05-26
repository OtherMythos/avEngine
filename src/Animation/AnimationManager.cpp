#include "AnimationManager.h"

#include "SequenceAnimationDef.h"
#include "System/BaseSingleton.h"

#include <cstring>
#include <Logger/Log.h>

#include "Animation/Script/AnimationScriptParser.h"
#include "AnimationInfoBlockUtil.h"
#include "System/Pause/PauseState.h"

namespace AV{
    AnimationManager::AnimationManager(){
    }

    AnimationManager::~AnimationManager(){

    }

    void AnimationManager::update(){
        if((PauseState::getMask() & PAUSE_TYPE_ANIMATIONS) > 0) return;

        for(void* id : mActiveAnimations){
            SequenceAnimation& anim = mAnimations.getEntry(id);
            //Should be running if it's in this list.
            assert(anim.running);
            bool stillRunning = anim.def->update(anim);
            if(!stillRunning){
                //Queue the removal for later so it doesn't interfere with the iteration.
                mQueuedEndAnimations.insert(id);
                anim.running = false;
            }
        }

        if(!mQueuedEndAnimations.empty()){
            for(void* id : mQueuedEndAnimations){
                mActiveAnimations.erase(id);
            }
            mQueuedEndAnimations.clear();
        }
    }

    void AnimationManager::addAnimationDefinitionsFromParser(const AnimationParserOutput& info){
        //NOTE if this is ever threaded, the pointers could be created on the thread, and then just added in later.
        for(size_t i = 0; i < info.animInfo.size(); i++){
            const std::string& targetName = info.animInfo[i].name;
            AV_INFO("Adding a parsed animation definition with name '{}'.", targetName);
            if(mAnimationDefs.find(targetName) != mAnimationDefs.end()){
                AV_WARN("An animation with the name '{}' already exists", targetName);
                continue;
            }
            SequenceAnimationDefPtr p = std::make_shared<SequenceAnimationDef>(i, info, targetName, this);
            mAnimationDefs[targetName] = p;
        }
    }

    SequenceAnimationDefPtr AnimationManager::createAnimationDefinition(const std::string& animName, const AnimationDefConstructionInfo& info){
        auto it = mAnimationDefs.find(animName);
        if(it != mAnimationDefs.end()){
            //An animation with that name already exists in the manager.
            return 0;
        }
        //Otherwise make a new one.
        SequenceAnimationDefPtr p = std::make_shared<SequenceAnimationDef>(info, animName, this);
        mAnimationDefs[animName] = p;
        return p;
    }

    SequenceAnimationDefPtr AnimationManager::getAnimationDefinition(const std::string& animName){
        auto it = mAnimationDefs.find(animName);
        if(it == mAnimationDefs.end()) return 0;

        SequenceAnimationDefPtr ptr = it->second;
        return ptr;
    }

    SequenceAnimationPtr AnimationManager::createAnimation(SequenceAnimationDefPtr def, AnimationInfoBlockPtr info, bool& success){
        AnimationInfoTypeHash hash = _getHashFromBlock(info.get());
        if(hash != def->getInfo().animInfoHash){
            success = false;
            return 0;
        }
        success = true;

        void* storedEntry = mAnimations.storeEntry({def, info, 0, true});
        mActiveAnimations.insert(storedEntry);

        SequenceAnimationPtr sharedPtr = SequenceAnimationPtr(storedEntry, _destroyAnimationInstance);
        return sharedPtr;
    }

    AnimationInfoBlockPtr AnimationManager::createAnimationInfoBlock(AnimationInfoEntry (&targetBlocks)[MAX_ANIMATION_INFO], uint8 numAnimationInfo, AnimationInfoTypeHash hash){
        //Right now I'm storing these on the heap for convenience. In future a custom memory management solution is possible.
        //I have to calculate how much size I need for each object, based on the intended contents.
        //I chose the malloc approach because it means I can vary the size of the object in memory based on how many items it needs.
        //I would otherwise need to have MAX_ANIMATION_INFO objects each time.
        size_t requiredSize = sizeof(AnimationInfoEntry) * numAnimationInfo;
        requiredSize += sizeof(uint8) * 1;
        requiredSize += sizeof(AnimationInfoTypeHash);
        void* target = malloc(requiredSize);
        assert(malloc);

        { //Populate the values in the new data.
            uint8* numValues = reinterpret_cast<uint8*>(target);
            *numValues = numAnimationInfo;
            numValues++;
            uint64* numValues64 = reinterpret_cast<uint64*>(numValues);
            *numValues64 = hash;
            numValues64++;
            AnimationInfoEntry* entryPtr = reinterpret_cast<AnimationInfoEntry*>(numValues64);
            for(uint8 i = 0; i < numAnimationInfo; i++){
                *entryPtr++ = targetBlocks[i];
            }
        }

        AnimationInfoBlockPtr targetPtr(target, _destroyAnimationInfoBlockInstance);
        return targetPtr;
    }

    void AnimationManager::setAnimationRunning(SequenceAnimationPtr p, bool running){
        if(!running){
            auto it = mActiveAnimations.find(p.get());
            if(it != mActiveAnimations.end()) mActiveAnimations.erase(it);
        }else{
            assert(mActiveAnimations.find(p.get()) == mActiveAnimations.end());
            mActiveAnimations.insert(p.get());
        }

        mAnimations.getEntry(p.get()).running = running;
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

    void AnimationManager::_removeCreatedAnimationDef(const std::string& animName){
        mAnimationDefs.erase(animName);
    }
}
