#pragma once

#include <vector>
#include <stack>
#include <set>

#include "CollisionWorldObject.h"
#include "CollisionWorldPrerequisites.h"

namespace AV{
    class CollisionWorldBruteForce : public CollisionWorldObject{
    public:
        CollisionWorldBruteForce(int worldId);
        ~CollisionWorldBruteForce();

        void processCollision();
        CollisionEntryId addCollisionPoint(float x, float y, float radius, uint8 mask=0xFF, CollisionEntryType collisionType=CollisionEntryType::either);
        CollisionEntryId addCollisionRectangle(float x, float y, float width, float height, uint8 mask=0xFF, CollisionEntryType collisionType=CollisionEntryType::either);
        CollisionEntryId addCollisionRotatedRectangle(float x, float y, float width, float height, float rotation, uint8 mask=0xFF, CollisionEntryType collisionType=CollisionEntryType::either);
        CollisionEntryId removeCollisionEntry(CollisionEntryId id);
        bool checkCollisionPoint(float x, float y, float radius, uint8 mask=0xFF);
        int getNumCollisions();
        CollisionPackedResult getCollisionPairForIdx(unsigned int idx);
        bool setPositionForPoint(CollisionEntryId idx, float x, float y);

    private:
        enum class BruteForceShape{
            NONE,
            CIRCLE,
            RECT,
            ROTATED_RECT
        };
        struct BruteForceEntry{
            BruteForceShape s;
            float x;
            float y;
            float rotation;
            union{
                struct{
                    float radius;
                }c;
                struct{
                    float width, height;
                }r;
            };
            uint8 mask;
            CollisionEntryType entryType = CollisionEntryType::either;
            bool hole = false;
            bool dirtyHole = false;
        };

        uint64 determineEnterLeaveBits(CollisionPackedResult first, CollisionPackedResult second, std::set<CollisionPackedResult>& prevPairs);

        bool checkCollision_(const BruteForceEntry& f, const BruteForceEntry& s) const;
        CollisionEntryId addEntry_(const BruteForceEntry& entry);

        std::vector<BruteForceEntry> mEntries;
        std::stack<size_t> mEntryHoles;
        std::vector<CollisionPackedResult> mCollisions;
        std::set<CollisionPackedResult> mPrevPairs;
        std::set<CollisionPackedResult> mPrevEnterLeavePairs;
    };
}
