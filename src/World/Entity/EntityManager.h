#pragma once

#include "entityx/entityx.h"

#include "World/Slot/SlotPosition.h"
#include "eId.h"

namespace AV{
    class OgreMeshManager;

    class EntityManager{
    public:
        EntityManager();
        ~EntityManager();

        void initialise();

        eId createEntity(SlotPosition pos);
        void destroyEntity(eId entity);

        void setEntityPosition(eId id, SlotPosition position);

        std::shared_ptr<OgreMeshManager> getMeshManager() { return mOgreMeshManager; }

    private:
        entityx::EntityX ex;

        std::shared_ptr<OgreMeshManager> mOgreMeshManager;

        inline entityx::Entity getEntityHandle(eId id){
            return entityx::Entity(&ex.entities, entityx::Entity::Id(id.id()));
        }

    };
}
