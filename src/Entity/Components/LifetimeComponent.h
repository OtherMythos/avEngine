#pragma once

#include "System/Timing/TimerManagerData.h"

namespace AV{
    struct LifetimeComponent {
        TimeValue remainingTime;

        LifetimeComponent(TimeValue lifetime) : remainingTime(lifetime) {}
    };
}
