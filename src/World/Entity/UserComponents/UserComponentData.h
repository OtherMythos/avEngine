#pragma once

namespace AV{
    typedef unsigned char ComponentType;
    //Describes which variables a component contains.
    typedef unsigned char ComponentCombination;

    typedef unsigned long long ComponentId;

    enum class ComponentDataTypes{
        NONE,
        BOOL,
        INT,
        FLOAT
    };

    static const int MAX_COMPONENT_DATA_TYPES = 4;
    static const int NUM_USER_COMPONENTS = 16;

    //TODO this will be updated in future when I set the values manually.
    static const int NUM_SET_USER_COMPONENTS = 6;

    union UserComponentDataEntry{
        bool b;
        int i;
        float f;
    };
}
