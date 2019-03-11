#pragma once

#include "entityx/entityx.h"

#include "World/Slot/SlotPosition.h"
#include "eId.h"

namespace AV{
    class EntityManager{
    public:
        EntityManager();
        ~EntityManager();

        void initialise();

        eId createEntity(SlotPosition pos);
        void destroyEntity(eId entity);

        void setEntityPosition(eId id, SlotPosition position);


    private:
        entityx::EntityX ex;

        inline entityx::Entity getEntityHandle(eId id){
            return entityx::Entity(&ex.entities, entityx::Entity::Id(id.id()));
        }

    };
}
