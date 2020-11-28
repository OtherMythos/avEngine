#pragma once

#include "System/EnginePrerequisites.h"
#include "UserComponentData.h"
#include <vector>
#include <string>

namespace AV{
    class UserComponentSettings{
    public:
        struct ComponentSetting{
            std::string componentName;
            ComponentCombination componentVars;
            ComponentType numVars;
        };
        ComponentSetting vars[NUM_USER_COMPONENTS];

        bool componentPopulated(ComponentType t) const{
            return vars[t].numVars > 0;
        }
    };

    class UserComponentManager{
    public:
        UserComponentManager();
        ~UserComponentManager();

        ComponentId createComponentOfType(ComponentType t);

        UserComponentDataEntry getValue(ComponentId t, ComponentType compType, uint8 varIdx);
        void setValue(ComponentId t, ComponentType compType, uint8 varIdx, UserComponentDataEntry value);

        static UserComponentSettings mSettings;

    private:
        struct UserComponentData1{
            UserComponentDataEntry e;
        };
        struct UserComponentData2{
            UserComponentDataEntry e[2];
        };
        struct UserComponentData3{
            UserComponentDataEntry e[3];
        };
        struct UserComponentData4{
            UserComponentDataEntry e[4];
        };
        std::vector<UserComponentData1> mComponentVec1;
        std::vector<UserComponentData2> mComponentVec2;
        std::vector<UserComponentData3> mComponentVec3;
        std::vector<UserComponentData4> mComponentVec4;

        ComponentCombination _dataTypesToCombination(const ComponentDataTypes (&data)[MAX_COMPONENT_DATA_TYPES]);
        void _combinationToDataTypes(ComponentCombination data, ComponentDataTypes (&outData)[MAX_COMPONENT_DATA_TYPES]);

        UserComponentDataEntry& _getDataForList(ComponentId t, ComponentType compType, uint8 varIdx);
    };
}
