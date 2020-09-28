#pragma once

#include "ComponentLogic.h"
#include "World/Nav/NavTypes.h"


namespace AV{
    class NavigationComponentLogic : public ComponentLogic{
    public:
        static bool add(eId id);
        static bool remove(eId id);
    };
}
