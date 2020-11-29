#pragma once

#include "System/EnginePrerequisites.h"
#include "UserComponentData.h"
#include <vector>
#include <string>

namespace AV{
    class UserComponentManager{
    public:
        UserComponentManager();
        ~UserComponentManager();

        ComponentId createComponentOfType(ComponentType t);
        void removeComponent(ComponentId id, ComponentType t);

        UserComponentDataEntry getValue(ComponentId t, ComponentType compType, uint8 varIdx);
        void setValue(ComponentId t, ComponentType compType, uint8 varIdx, UserComponentDataEntry value);

        uint32 getNumRegisteredComponents() const { return mNumRegisteredComponents; }

    private:
        struct UserComponentData1{
            bool populated;
            UserComponentDataEntry e;
        };
        struct UserComponentData2{
            bool populated;
            UserComponentDataEntry e[2];
        };
        struct UserComponentData3{
            bool populated;
            UserComponentDataEntry e[3];
        };
        struct UserComponentData4{
            bool populated;
            UserComponentDataEntry e[4];
        };
        std::vector<UserComponentData1> mComponentVec1;
        std::vector<UserComponentData2> mComponentVec2;
        std::vector<UserComponentData3> mComponentVec3;
        std::vector<UserComponentData4> mComponentVec4;

        uint32 mNumRegisteredComponents = 0;

        UserComponentDataEntry& _getDataForList(ComponentId t, ComponentType compType, uint8 varIdx);
    };
}
