#pragma once

namespace AV{
    typedef unsigned char ComponentType;
    typedef unsigned char ComponentCombination;

    typedef unsigned long long ComponentId;

    enum class ComponentDataTypes{
        BOOL,
        INT,
        FLOAT
    };

    static const int MAX_COMPONENT_DATA_TYPES = 4;
}
