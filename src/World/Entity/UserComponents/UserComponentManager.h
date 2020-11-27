#pragma once

#include "System/EnginePrerequisites.h"
#include "UserComponentData.h"
#include <vector>

namespace AV{
    class UserComponentManager{
    public:
        UserComponentManager();
        ~UserComponentManager();

        ComponentId createComponentOfType(ComponentType t);

        UserComponentDataEntry getValue(ComponentId t, uint8 listId, uint8 varIdx);
        void setValue(ComponentId t, uint8 listId, uint8 varIdx, UserComponentDataEntry value);

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

        UserComponentDataEntry& _getDataForList(ComponentId t, uint8 listId, uint8 varIdx);
    };
}
