#pragma once

#include <vector>
#include <memory>
#include <set>
#include <stack>
#include "CollisionWorldObject.h"
#include "CollisionWorldPrerequisites.h"

namespace AV {

    // Forward declaration
    class CollisionOctree;

    struct AABB {
        float minX, minY, maxX, maxY;

        AABB() : minX(0), minY(0), maxX(0), maxY(0) {}
        AABB(float minX, float minY, float maxX, float maxY)
            : minX(minX), minY(minY), maxX(maxX), maxY(maxY) {}

        bool contains(float x, float y) const {
            return x >= minX && x <= maxX && y >= minY && y <= maxY;
        }

        bool intersects(const AABB& other) const {
            return !(maxX < other.minX || minX > other.maxX ||
                     maxY < other.minY || minY > other.maxY);
        }
    };

    enum class CollisionShape {
        NONE,
        CIRCLE,
        RECTANGLE
    };

    struct CollisionEntry {
        CollisionEntry() :
            shape(CollisionShape::CIRCLE),
            x(0), y(0), c{0},
            entryType(CollisionEntryType::either),
            hole(false), dirtyHole(false) {}

        CollisionEntry(CollisionShape shape, float x, float y, uint8 mask, CollisionEntryType entryType, float radius, float width, float height) :
            shape(shape),
            x(x), y(y),
            r{width, height},
            entryType(entryType),
            hole(false), dirtyHole(false), id(0) {}

        CollisionShape shape;
        float x, y;
        uint8 mask;
        CollisionEntryType entryType;
        union{
            struct{
                float radius;
            }c;
            struct{
                float width, height;
            }r;
        };
        CollisionEntryId id;

        bool hole = false;
        bool dirtyHole = false;

        AABB getBounds() const {
            if (shape == CollisionShape::CIRCLE) {
                return AABB(x - c.radius, y - c.radius, x + c.radius, y + c.radius);
            } else if (shape == CollisionShape::RECTANGLE) {
                return AABB(x, y, x + r.width, y + r.height);
            }
            return AABB();
        }
    };

    class CollisionWorldOctree : public CollisionWorldObject {
    public:
        CollisionWorldOctree(int worldId, const AABB& bounds = AABB(-1000, -1000, 1000, 1000), int maxDepth = 8);
        ~CollisionWorldOctree();

        void processCollision() override;
        CollisionEntryId addCollisionPoint(float x, float y, float radius, uint8 mask = 0xFF, CollisionEntryType collisionType = CollisionEntryType::either) override;
        CollisionEntryId addCollisionRectangle(float x, float y, float width, float height, uint8 mask = 0xFF, CollisionEntryType collisionType = CollisionEntryType::either) override;
        CollisionEntryId removeCollisionEntry(CollisionEntryId entryId) override;
        bool checkCollisionPoint(float x, float y, float radius) override;
        int getNumCollisions() override;
        CollisionPackedResult getCollisionPairForIdx(unsigned int idx) override;
        bool setPositionForPoint(CollisionEntryId entryId, float x, float y) override;

        uint64 determineEnterLeaveBits(CollisionPackedResult first, CollisionPackedResult second, std::set<CollisionPackedResult>& prevPairs);

    private:
        bool checkCollision_(const CollisionEntry& a, const CollisionEntry& b) const;

        std::vector<CollisionEntry> mEntries;
        std::vector<CollisionPackedResult> mCollisions;
        std::set<CollisionPackedResult> mPrevPairs;
        std::set<CollisionPackedResult> mPrevEnterLeavePairs;
        std::stack<size_t> mEntryHoles;
        class CollisionOctree {
        public:
            CollisionOctree(const AABB& bounds = AABB(), int maxDepth = -1);
            ~CollisionOctree();

            void insert(CollisionEntryId id, std::vector<CollisionEntry>& totalEntries, const CollisionEntry& entry);
            void remove(CollisionEntryId id, const CollisionEntry& entry);
            void query(const CollisionEntry& entry, std::vector<CollisionEntry>& totalEntries, std::vector<CollisionEntry*>& results);

        private:
            static const int MAX_OBJECTS = 8;

            AABB mBounds;
            int mMaxDepth;
            int mDepth;
            bool mIsLeaf;
            std::vector<CollisionEntryId> mObjectIds;
            std::vector<CollisionOctree> mChildren;

            void split(std::vector<CollisionEntry>& totalEntries);
            int getQuadrant(const AABB& bounds) const;
        };

        CollisionEntryId addEntry_(CollisionEntry& entry);

        CollisionOctree mRoot;
    };
}
