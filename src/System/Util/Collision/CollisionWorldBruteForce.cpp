#include "CollisionWorldBruteForce.h"

#include "System/EnginePrerequisites.h"
#include <cassert>

#include <cassert>

namespace AV{

    CollisionWorldBruteForce::CollisionWorldBruteForce(int worldId) : CollisionWorldObject(worldId){

    }

    CollisionWorldBruteForce::~CollisionWorldBruteForce(){

    }

    void CollisionWorldBruteForce::processCollision(){
        mCollisions.clear();
        std::set<CollisionPackedResult> pairs;
        std::set<CollisionPackedResult> enterLeavePairs;

        //TODO have another think about the dirty points.
        //Might help when processing the entered and left system.

        //for(CollisionEntryId testerId : mDirtyPoints){
        for(int y = 0; y < mEntries.size(); y++){
            BruteForceEntry& tester = mEntries[y];
            if(tester.hole){
                if(tester.dirtyHole){
                    tester.dirtyHole = false;
                    mEntryHoles.push(y);
                }
                continue;
            }
            for(int i = 0; i < mEntries.size(); i++){
                if(i == y) continue;
                const BruteForceEntry& check = mEntries[i];
                if(check.hole) continue;
                if((check.mask & tester.mask) == 0) continue;

                uint64 targetFirstId = y;
                uint64 targetSecondId = i;
                if(check.entryType != CollisionEntryType::either || tester.entryType != CollisionEntryType::either){
                    if(
                       check.entryType == CollisionEntryType::sender && tester.entryType == CollisionEntryType::receiver)
                    {
                        targetFirstId = i;
                        targetSecondId = y;
                    }
                    else if(check.entryType == CollisionEntryType::receiver && tester.entryType == CollisionEntryType::sender)
                    {
                        targetFirstId = y;
                        targetSecondId = i;
                    }else{
                       //Just ignore it if it does't match up correctly.
                       continue;
                    }
                }
                bool result = checkCircleCollision(tester.x, tester.y, tester.radius, check.x, check.y, check.radius);
                if(result){
                    CollisionPackedResult first = static_cast<uint64>(targetFirstId) << 30 | targetSecondId;
                    CollisionPackedResult second = static_cast<uint64>(targetSecondId) << 30 | targetFirstId;
                    if(pairs.find(first) == pairs.end() && pairs.find(second) == pairs.end()){
                        CollisionPackedResult outResult = second;
                        enterLeavePairs.insert(outResult);
                        outResult |= determineEnterLeaveBits(first, second, mPrevPairs) << 60;
                        mCollisions.push_back(outResult);
                    }
                    pairs.insert(first);
                    pairs.insert(second);
                }
            }
        }

        for(CollisionPackedResult i : mPrevEnterLeavePairs){
            if(enterLeavePairs.find(i) != enterLeavePairs.end()) continue;
            //Register that this point left.
            CollisionPackedResult out = i | (static_cast<CollisionPackedResult>(0x2) << 60);
            mCollisions.push_back(out);
        }

        mPrevPairs = enterLeavePairs;
        mPrevEnterLeavePairs = enterLeavePairs;
    }

    bool CollisionWorldBruteForce::checkCollisionPoint(float x, float y, float radius){
        for(int i = 0; i < mEntries.size(); i++){
            const BruteForceEntry& tester = mEntries[i];
            if(tester.hole) continue;
            if(checkCircleCollision(tester.x, tester.y, tester.radius, x, y, radius)){
                return true;
            }
        }

        return false;
    }

    uint64 CollisionWorldBruteForce::determineEnterLeaveBits(CollisionPackedResult first, CollisionPackedResult second, std::set<CollisionPackedResult>& prevPairs) {
        auto firstIt = prevPairs.find(first);
        auto secondIt = prevPairs.find(second);
        uint64 out = 0;
        if(firstIt != prevPairs.end()){
            //The collision is active.
            prevPairs.erase(firstIt);
        }
        else if(secondIt != prevPairs.end()){
            prevPairs.erase(secondIt);
        }else{
            //The collision just began.
            out |= 0x1;
        }

        return out;
    }

    CollisionEntryId CollisionWorldBruteForce::addCollisionPoint(float x, float y, float radius, uint8 mask, CollisionEntryType collisionType){
        BruteForceEntry entry{x, y, radius, mask, collisionType};

        CollisionEntryId targetIdx = COLLISION_ENTRY_ID_INVALID;
        if(mEntryHoles.empty()){
            size_t id = mEntries.size();
            mEntries.push_back(entry);
            targetIdx = static_cast<CollisionEntryId>(id);
        }else{
            size_t idx = mEntryHoles.top();
            mEntryHoles.pop();
            assert(mEntries[idx].hole == true);
            assert(mEntries[idx].dirtyHole == false);
            mEntries[idx] = entry;
            targetIdx = static_cast<CollisionEntryId>(idx);
        }
        assert(targetIdx != COLLISION_ENTRY_ID_INVALID);
        //mDirtyPoints.insert(targetIdx);

        return targetIdx;
    }

    CollisionEntryId CollisionWorldBruteForce::removeCollisionPoint(CollisionEntryId id){
        assert(id < mEntries.size());
        assert(mEntries[id].hole == false);
        mEntries[id].hole = true;
        mEntries[id].dirtyHole = true;

        return 0;
    }

    int CollisionWorldBruteForce::getNumCollisions(){
        return static_cast<int>(mCollisions.size());
    }

    CollisionPackedResult CollisionWorldBruteForce::getCollisionPairForIdx(unsigned int idx){
        if(idx >= mCollisions.size()) return COLLISION_PACKED_RESULT_INVALID;

        return mCollisions[idx];
    }

    bool CollisionWorldBruteForce::setPositionForPoint(CollisionEntryId id, float x, float y){
        if(id >= mEntries.size()) return false;
        if(mEntries[id].hole == true) return false;
        mEntries[id].x = x;
        mEntries[id].y = y;

        //mDirtyPoints.insert(id);

        return true;
    }


}
