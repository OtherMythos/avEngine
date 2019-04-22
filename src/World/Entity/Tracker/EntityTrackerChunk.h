#pragma once

#include "World/Entity/eId.h"
#include <set>

namespace AV{
    class EntityManager;

    class EntityTrackerChunk{
    public:
        EntityTrackerChunk();
        virtual ~EntityTrackerChunk();

        virtual void addEntity(eId e);
        bool removeEntity(eId e);

        bool containsEntity(eId e);
        void destroyChunk(EntityManager* entityManager);

        int getEntityCount() { return mEntities.size(); }

    private:
        std::set<eId> mEntities;
    };
}
