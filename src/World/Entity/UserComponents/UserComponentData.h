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
    static const int NUM_USER_COMPONENTS = 16;

    //TODO this will be updated in future when I set the values manually.
    static const int NUM_SET_USER_COMPONENTS = 6;
}
