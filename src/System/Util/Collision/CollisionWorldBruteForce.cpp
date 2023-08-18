#include "CollisionWorldBruteForce.h"

#include "System/EnginePrerequisites.h"

namespace AV{

    CollisionWorldBruteForce::CollisionWorldBruteForce(){

    }

    CollisionWorldBruteForce::~CollisionWorldBruteForce(){

    }

    void CollisionWorldBruteForce::processCollision(){
        mCollisions.clear();
        std::set<CollisionPackedResult> pairs;

        for(CollisionEntryId testerId : mDirtyPoints){
            const BruteForceEntry& tester = mEntries[testerId];
            for(int i = 0; i < mEntries.size(); i++){
                if(i == testerId) continue;
                const BruteForceEntry& check = mEntries[i];
                if(check.hole) continue;
                if((check.mask & tester.mask) == 0) continue;
                bool result = checkCircleCollision(tester.x, tester.y, tester.radius, check.x, check.y, check.radius);
                if(result){

                    CollisionPackedResult first = static_cast<uint64>(testerId) << 32 | i;
                    CollisionPackedResult second = static_cast<uint64>(i) << 32 | testerId;
                    if(pairs.find(first) == pairs.end() && pairs.find(second) == pairs.end()){

                        mCollisions.push_back(result);
                    }
                    pairs.insert(first);
                    pairs.insert(second);
                }
            }
        }
    }

    CollisionEntryId CollisionWorldBruteForce::addCollisionPoint(float x, float y, float radius, uint8 mask){
        BruteForceEntry entry{x, y, radius, mask};

        CollisionEntryId targetIdx = COLLISION_ENTRY_ID_INVALID;
        if(mEntryHoles.empty()){
            size_t id = mEntries.size();
            mEntries.push_back(entry);
            targetIdx = static_cast<CollisionEntryId>(id);
        }else{
            size_t idx = mEntryHoles.top();
            mEntryHoles.pop();
            assert(mEntries[idx].hole == true);
            mEntries[idx] = entry;
            targetIdx = static_cast<CollisionEntryId>(idx);
        }
        assert(targetIdx != COLLISION_ENTRY_ID_INVALID);
        mDirtyPoints.insert(targetIdx);

        return targetIdx;
    }

    CollisionEntryId CollisionWorldBruteForce::removeCollisionPoint(CollisionEntryId id){
        assert(id < mEntries.size());
        assert(mEntries[id].hole == false);
        mEntries[id].hole = true;
        mEntryHoles.push(id);
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

        mDirtyPoints.insert(id);

        return true;
    }


}
