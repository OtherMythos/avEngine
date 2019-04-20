#pragma once

#include "entityx/entityx.h"

#include "World/Slot/SlotPosition.h"
#include "eId.h"

namespace AV{
    class OgreMeshManager;
    class EntityTracker;
    class Event;

    class EntityManager{
    public:
        struct EntityDebugInfo{
            int totalEntities;
            int trackedEntities;
            int trackingChunks;
        };

    public:
        EntityManager();
        ~EntityManager();

        void initialise();

        eId createEntity(SlotPosition pos);
        eId createEntityTracked(SlotPosition pos);
        
        void destroyKnownEntity(eId entity, bool tracked);
        void destroyEntity(eId entity);
        
        bool worldEventReceiver(const Event &e);

        bool getEntityValid(eId entity);

        void setEntityPosition(eId id, SlotPosition position);

        std::shared_ptr<OgreMeshManager> getMeshManager() { return mOgreMeshManager; }
        std::shared_ptr<EntityTracker> getEntityTracker() { return mEntityTracker; }

        void getDebugInfo(EntityDebugInfo *info);

    private:
        entityx::EntityX ex;

        std::shared_ptr<OgreMeshManager> mOgreMeshManager;
        std::shared_ptr<EntityTracker> mEntityTracker;

        entityx::Entity _createEntity(SlotPosition pos, bool tracked);
        
        void _repositionEntityOriginSwitch();
        void _mapChange();

        inline entityx::Entity getEntityHandle(eId id){
            return entityx::Entity(&ex.entities, entityx::Entity::Id(id.id()));
        }

    };
}
