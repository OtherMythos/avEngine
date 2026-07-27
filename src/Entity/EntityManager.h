#pragma once

#include "entityx/entityx.h"

#include "OgreVector3.h"
#include "eId.h"
#include "Callback/EntityCallbackEvents.h"

namespace AV{
    class Event;
    class EntityCallbackManager;
    class EntitySerialisationJob;
    class PhysicsManager;
    class UserComponentManager;

    class EntityManager{
        friend class EntitySerialisationJob;
    public:
        struct EntityDebugInfo{
            int totalEntities;
            int totalCallbackScripts;
        };

    public:
        EntityManager();
        ~EntityManager();

        void initialise();

        void update();

        eId createEntity(Ogre::Vector3 pos);

        void destroyKnownEntity(eId entity);
        void destroyEntity(eId entity);


        bool getEntityValid(eId entity);

        /**
        Set the position of an entity.

        @param autoMov
        Whether or not this movement is being performed by a facet of the entity, for instance an attached rigid body setting the entity position.
        */
        void setEntityPosition(eId id, Ogre::Vector3 position, bool autoMove = false);
        void setEntityOrientation(eId id, Ogre::Quaternion orientation);

        void notifyEntityEvent(eId entity, EntityEventType event);

        std::shared_ptr<EntityCallbackManager> getEntityCallbackManager() { return mEntityCallbackManager; }
        std::shared_ptr<UserComponentManager> getUserComponentManager() { return mUserComponentManager; }

        void setPhysicsManager(std::shared_ptr<PhysicsManager> manager) { mPhysicsManager = manager; }

        void getDebugInfo(EntityDebugInfo *info);

    private:
        entityx::EntityX ex;

        std::shared_ptr<EntityCallbackManager> mEntityCallbackManager;
        std::shared_ptr<UserComponentManager> mUserComponentManager;

        std::shared_ptr<PhysicsManager> mPhysicsManager;

        entityx::Entity _createEntity(Ogre::Vector3 pos);


        inline entityx::Entity getEntityHandle(eId id){
            return entityx::Entity(&ex.entities, entityx::Entity::Id(id.id()));
        }

    };
}
