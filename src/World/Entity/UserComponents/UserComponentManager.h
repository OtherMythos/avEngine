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

        bool getBool(ComponentId t, uint8 listId, uint8 varIdx);
        void setBool(ComponentId t, uint8 listId, uint8 varIdx, bool value);

    private:
        union DataEntry{
            bool b;
            int i;
            float f;
        };
        struct UserComponentData1{
            DataEntry e;
        };
        struct UserComponentData2{
            DataEntry e[2];
        };
        struct UserComponentData3{
            DataEntry e[3];
        };
        struct UserComponentData4{
            DataEntry e[4];
        };
        std::vector<UserComponentData1> mComponentVec1;
        std::vector<UserComponentData2> mComponentVec2;
        std::vector<UserComponentData3> mComponentVec3;
        std::vector<UserComponentData4> mComponentVec4;

        ComponentCombination _dataTypesToCombination(const ComponentDataTypes (&data)[MAX_COMPONENT_DATA_TYPES]);
        void _combinationToDataTypes(ComponentCombination data, ComponentDataTypes (&outData)[MAX_COMPONENT_DATA_TYPES]);

        DataEntry& _getDataForList(ComponentId t, uint8 listId, uint8 varIdx);
    };
}
