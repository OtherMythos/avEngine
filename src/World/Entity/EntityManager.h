#pragma once

#include "entityx/entityx.h"

#include "World/Slot/SlotPosition.h"
#include "eId.h"
#include "Callback/EntityCallbackEvents.h"

namespace AV{
    class OgreMeshManager;
    class EntityTracker;
    class Event;
    class EntityCallbackManager;

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
        
        void notifyEntityEvent(eId entity, EntityEventType event);

        std::shared_ptr<OgreMeshManager> getMeshManager() { return mOgreMeshManager; }
        std::shared_ptr<EntityTracker> getEntityTracker() { return mEntityTracker; }
        std::shared_ptr<EntityCallbackManager> getEntityCallbackManager() { return mEntityCallbackManager; }

        void getDebugInfo(EntityDebugInfo *info);

    private:
        entityx::EntityX ex;

        std::shared_ptr<OgreMeshManager> mOgreMeshManager;
        std::shared_ptr<EntityTracker> mEntityTracker;
        std::shared_ptr<EntityCallbackManager> mEntityCallbackManager;

        entityx::Entity _createEntity(SlotPosition pos, bool tracked);
        
        void _repositionEntityOriginSwitch();
        void _mapChange();

        inline entityx::Entity getEntityHandle(eId id){
            return entityx::Entity(&ex.entities, entityx::Entity::Id(id.id()));
        }

    };
}
