#pragma once

#include "Nav/NavTypes.h"

namespace AV{
    struct NavigationComponent {
        NavigationComponent() {}
        NavQueryId queryId = INVALID_NAV_QUERY;
        bool findingPath = false;
        float targetSpeed = 1.0f;
    };
}
