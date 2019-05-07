#pragma once

#include "World/Entity/eId.h"

namespace entityx{
    class EntityX;
    class Entity;
}

namespace AV{
    class EntityManager;

    class ComponentLogic{
        friend EntityManager;

    protected:
        static EntityManager* entityManager;
        static entityx::EntityX* entityXManager;
    };
}
