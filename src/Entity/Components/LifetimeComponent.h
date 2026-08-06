#pragma once

#include "System/EnginePrerequisites.h"

namespace AV{
    struct LifetimeComponent {
        uint64 remainingTime;

        LifetimeComponent(uint64 lifetime) : remainingTime(lifetime) {}
    };
}
