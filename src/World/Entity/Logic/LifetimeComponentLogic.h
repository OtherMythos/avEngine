#pragma once

#include "ComponentLogic.h"
#include "World/Entity/Components/LifetimeComponent.h"

namespace AV{

    class LifetimeComponentLogic : public ComponentLogic{
    public:
        static void add(eId id, TimeValue lifetime);
        static bool remove(eId id);
    };
}
