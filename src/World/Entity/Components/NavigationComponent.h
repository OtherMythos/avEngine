#pragma once

#include "World/Nav/NavTypes.h"

namespace AV{
    struct NavigationComponent {
        NavigationComponent(NavQueryId id)
            : queryId(id) {}
        NavQueryId queryId = INVALID_NAV_QUERY;
    };
}
