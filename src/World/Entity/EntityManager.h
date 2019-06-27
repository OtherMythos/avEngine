#pragma once

#include "entityx/entityx.h"

#include "World/Slot/SlotPosition.h"
#include "eId.h"
#include "Callback/EntityCallbackEvents.h"

namespace AV{
    class EntityTracker;
    class Event;
    class EntityCallbackManager;
    class EntitySerialisationJob;
    class PhysicsManager;

    class EntityManager{
        friend class EntitySerialisationJob;
    public:
        struct EntityDebugInfo{
            int totalEntities;
            int trackedEntities;
            int trackingChunks;
            int totalCallbackScripts;
        };

    public:
        EntityManager();
        ~EntityManager();

        void initialise();

        void update();

        eId createEntity(SlotPosition pos);
        eId createEntityTracked(SlotPosition pos);

        void destroyKnownEntity(eId entity, bool tracked);
        void destroyEntity(eId entity);

        bool worldEventReceiver(const Event &e);

        bool getEntityValid(eId entity);

        void setEntityPosition(eId id, SlotPosition position);
        void setEntityOrientation(eId id, Ogre::Quaternion orientation);

        void notifyEntityEvent(eId entity, EntityEventType event);

        std::shared_ptr<EntityTracker> getEntityTracker() { return mEntityTracker; }
        std::shared_ptr<EntityCallbackManager> getEntityCallbackManager() { return mEntityCallbackManager; }

        void setPhysicsManager(std::shared_ptr<PhysicsManager> manager) { mPhysicsManager = manager; }

        void getDebugInfo(EntityDebugInfo *info);

    private:
        entityx::EntityX ex;

        std::shared_ptr<EntityTracker> mEntityTracker;
        std::shared_ptr<EntityCallbackManager> mEntityCallbackManager;

        std::shared_ptr<PhysicsManager> mPhysicsManager;

        entityx::Entity _createEntity(SlotPosition pos, bool tracked);

        void _repositionEntityOriginSwitch();
        void _mapChange();

        inline entityx::Entity getEntityHandle(eId id){
            return entityx::Entity(&ex.entities, entityx::Entity::Id(id.id()));
        }

    };
}
