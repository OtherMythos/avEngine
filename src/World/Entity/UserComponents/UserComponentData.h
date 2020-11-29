#pragma once

#include <string>
#include "System/EnginePrerequisites.h"

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

    union UserComponentDataEntry{
        bool b;
        int i;
        float f;
    };

    class UserComponentSettings{
    public:
        struct ComponentSetting{
            std::string componentName;
            ComponentCombination componentVars;
            ComponentType numVars;
        };
        ComponentSetting vars[NUM_USER_COMPONENTS];
        uint8 numRegisteredComponents;

        bool componentPopulated(ComponentType t) const{
            return vars[t].numVars > 0;
        }
    };

    //TODO remove the _

    static ComponentDataTypes getTypeOfVariable(ComponentCombination c, uint8 var){
        return static_cast<ComponentDataTypes>( (c >> var * 2) & 0x3 );
    }

    static ComponentCombination _dataTypesToCombination(const ComponentDataTypes (&data)[4]){
        ComponentCombination c = 0;

        for(uint8 i = 0; i < MAX_COMPONENT_DATA_TYPES; i++){
            ComponentDataTypes d = data[i];
            c |= static_cast<unsigned char>(d) << (i * 2);
        }
        return c;
    }

    static void _combinationToDataTypes(ComponentCombination data, ComponentDataTypes (&outData)[MAX_COMPONENT_DATA_TYPES]){
        for(uint8 i = 0; i < MAX_COMPONENT_DATA_TYPES; i++){
            outData[i] = getTypeOfVariable(data, i);
        }
    }
}
