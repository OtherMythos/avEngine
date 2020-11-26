#pragma once

#include "ComponentLogic.h"
#include "World/Entity/UserComponents/UserComponentData.h"

namespace AV{

    class UserComponentLogic : public ComponentLogic{
    public:
        UserComponentLogic() = delete;
        ~UserComponentLogic() = delete;

        static void add(eId id, ComponentType t, ComponentId comp);
        static void remove(eId id, ComponentType t);
    };
}
