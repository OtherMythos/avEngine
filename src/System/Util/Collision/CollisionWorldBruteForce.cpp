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
            if(tester.entryType == CollisionEntryType::receiver) continue;

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
                bool result = checkCollision_(tester, check);
                if(result){
                    CollisionPackedResult first = static_cast<uint64>(targetFirstId) << 30 | targetSecondId;
                    CollisionPackedResult second = static_cast<uint64>(targetSecondId) << 30 | targetFirstId;
                    if(pairs.find(first) == pairs.end() && pairs.find(second) == pairs.end()){
                        CollisionPackedResult outResult = second;
                        enterLeavePairs.insert(outResult);
                        outResult |= CollisionWorldObject::determineEnterLeaveBits(first, second, mPrevPairs) << 60;
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

    bool CollisionWorldBruteForce::checkCollision_(const BruteForceEntry& f, const BruteForceEntry& s) const{
        if(f.s == BruteForceShape::CIRCLE && s.s == BruteForceShape::CIRCLE){
            return checkCircleCollision(f.x, f.y, f.c.radius, s.x, s.y, s.c.radius);
        }
        else if(f.s == BruteForceShape::RECT && s.s == BruteForceShape::RECT){
            return checkRectangleCollision(f.x, f.y, f.r.width, f.r.height, s.x, s.y, s.r.width, s.r.height);
        }else if(f.s == BruteForceShape::CIRCLE && s.s == BruteForceShape::ROTATED_RECT){
            return checkCircleRotatedRectangleCollision(f.x, f.y, f.c.radius, s.x, s.y, s.r.width, s.r.height, s.rotation);
        }else if(f.s == BruteForceShape::ROTATED_RECT && s.s == BruteForceShape::CIRCLE){
            return checkCircleRotatedRectangleCollision(s.x, s.y, s.c.radius, f.x, f.y, f.r.width, f.r.height, f.rotation);
        }else if(f.s == BruteForceShape::RECT && s.s == BruteForceShape::ROTATED_RECT){
            return checkRotatedRectangleCollision(f.x, f.y, f.r.width, f.r.height, 0, s.x, s.y, s.r.width, s.r.height, s.rotation);
        }else if(f.s == BruteForceShape::ROTATED_RECT && s.s == BruteForceShape::RECT){
            return checkRotatedRectangleCollision(f.x, f.y, f.r.width, f.r.height, f.rotation, s.x, s.y, s.r.width, s.r.height, 0);
        }else if(f.s == BruteForceShape::ROTATED_RECT && s.s == BruteForceShape::ROTATED_RECT){
            return checkRotatedRectangleCollision(f.x, f.y, f.r.width, f.r.height, f.rotation, s.x, s.y, s.r.width, s.r.height, s.rotation);
        }else{
            if(f.s == BruteForceShape::CIRCLE){
                assert(s.s == BruteForceShape::RECT);
                return checkCircleRectangleCollision(f.x, f.y, f.c.radius, s.x, s.y, s.r.width, s.r.height);
            }
            else if(s.s == BruteForceShape::CIRCLE){
                assert(f.s == BruteForceShape::RECT);
                return checkCircleRectangleCollision(s.x, s.y, s.c.radius, f.x, f.y, f.r.width, f.r.height);
            }
        }
        return false;
    }

    bool CollisionWorldBruteForce::checkCollisionPoint(float x, float y, float radius){
        BruteForceEntry val;
        val.s = BruteForceShape::CIRCLE;
        val.x = x;
        val.y = y;
        val.rotation = 0;
        val.c.radius = radius;
        val.mask = 0xFF;
        val.entryType = CollisionEntryType::either;

        for(int i = 0; i < mEntries.size(); i++){
            const BruteForceEntry& tester = mEntries[i];
            if(tester.hole) continue;
            if(checkCollision_(tester, val)){
                return true;
            }
        }

        return false;
    }

    CollisionEntryId CollisionWorldBruteForce::addCollisionPoint(float x, float y, float radius, uint8 mask, CollisionEntryType collisionType){
        BruteForceEntry entry;
        entry.s = BruteForceShape::CIRCLE;
        entry.x = x;
        entry.y = y;
        entry.rotation = 0;
        entry.c.radius = radius;
        entry.mask = mask;
        entry.entryType = collisionType;

        return addEntry_(entry);
    }

    CollisionEntryId CollisionWorldBruteForce::addCollisionRectangle(float x, float y, float width, float height, uint8 mask, CollisionEntryType collisionType){
        BruteForceEntry entry;
        entry.s = BruteForceShape::RECT;
        entry.x = x;
        entry.y = y;
        entry.rotation = 0;
        entry.r.width = width;
        entry.r.height = height;
        entry.mask = mask;
        entry.entryType = collisionType;

        return addEntry_(entry);
    }

    CollisionEntryId CollisionWorldBruteForce::addCollisionRotatedRectangle(float x, float y, float width, float height, float rotation, uint8 mask, CollisionEntryType collisionType){
        BruteForceEntry entry;
        entry.s = BruteForceShape::ROTATED_RECT;
        entry.x = x;
        entry.y = y;
        entry.rotation = rotation;
        entry.r.width = width;
        entry.r.height = height;
        entry.mask = mask;
        entry.entryType = collisionType;

        return addEntry_(entry);
    }

    CollisionEntryId CollisionWorldBruteForce::addEntry_(const BruteForceEntry& entry){
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

        return targetIdx;

    }

    CollisionEntryId CollisionWorldBruteForce::removeCollisionEntry(CollisionEntryId id){
        assert(id < mEntries.size());
        assert(mEntries[id].hole == false);
        mEntries[id].hole = true;
        mEntries[id].dirtyHole = true;
        mEntries[id].s = BruteForceShape::NONE;

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
