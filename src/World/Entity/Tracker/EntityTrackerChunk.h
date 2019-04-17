#pragma once

#include "World/Entity/eId.h"
#include <set>

namespace AV{
    class EntityTrackerChunk{
    public:
        EntityTrackerChunk();
        ~EntityTrackerChunk();
        
        void addEntity(eId e);
        bool removeEntity(eId e);
        
        bool containsEntity(eId e);
        void destroyChunk();
        
    private:
        std::set<eId> mEntities;
    };
}
