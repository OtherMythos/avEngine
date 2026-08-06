#pragma once

#include "ComponentLogic.h"
#include "Entity/Components/LifetimeComponent.h"

namespace AV{

    class LifetimeComponentLogic : public ComponentLogic{
    public:
        static void add(eId id, uint64 lifetime);
        static bool remove(eId id);
    };
}
