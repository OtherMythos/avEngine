#pragma once

#include "ComponentLogic.h"
#include "World/Entity/UserComponents/UserComponentData.h"
#include "System/EnginePrerequisites.h"

namespace AV{

    class UserComponentLogic : public ComponentLogic{
    public:
        UserComponentLogic() = delete;
        ~UserComponentLogic() = delete;

        enum ErrorTypes{
            SUCCESS,
            NO_COMPONENT,
            COMPONENT_NOT_POPULATED
        };

        static ErrorTypes add(eId id, ComponentType t);
        static ErrorTypes remove(eId id, ComponentType t);
        static ErrorTypes set(eId id, ComponentType t, uint8 varId, UserComponentDataEntry e);
        static UserComponentLogic::ErrorTypes get(eId id, ComponentType t, uint8 varId, UserComponentDataEntry* e);
    };
}
