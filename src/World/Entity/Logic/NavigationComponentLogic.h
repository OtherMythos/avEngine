#pragma once

#include "ComponentLogic.h"
#include "World/Nav/NavTypes.h"
#include "World/Slot/SlotPosition.h"

namespace AV{
    class NavigationComponentLogic : public ComponentLogic{
    public:
        static bool add(eId id);
        static bool remove(eId id);

        static bool navigateTo(eId id, const SlotPosition& pos);
    };
}
