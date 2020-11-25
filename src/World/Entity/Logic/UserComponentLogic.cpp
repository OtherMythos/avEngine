#include "UserComponentLogic.h"

#include "entityx/entityx.h"

namespace AV{

    struct UserComponent{
        int id;
    };
    struct UserComponent0 : public UserComponent{ };
    struct UserComponent1 : public UserComponent{ };
    struct UserComponent2 : public UserComponent{ };
    struct UserComponent3 : public UserComponent{ };
    struct UserComponent4 : public UserComponent{ };
    struct UserComponent5 : public UserComponent{ };
    struct UserComponent6 : public UserComponent{ };
    struct UserComponent7 : public UserComponent{ };
    struct UserComponent8 : public UserComponent{ };
    struct UserComponent9 : public UserComponent{ };
    struct UserComponent10 : public UserComponent{ };
    struct UserComponent11 : public UserComponent{ };
    struct UserComponent12 : public UserComponent{ };
    struct UserComponent13 : public UserComponent{ };
    struct UserComponent14 : public UserComponent{ };
    struct UserComponent15 : public UserComponent{ };


    #define COMPONENT_SWITCH(EE, AA, ARGS, RET) switch(AA){ \
            case 0: RET EE.COMPONENT_FUNCTION<UserComponent0>(ARGS); break; \
            case 1: RET EE.COMPONENT_FUNCTION<UserComponent1>(ARGS); break; \
            case 2: RET EE.COMPONENT_FUNCTION<UserComponent2>(ARGS); break; \
            case 3: RET EE.COMPONENT_FUNCTION<UserComponent3>(ARGS); break; \
            case 4: RET EE.COMPONENT_FUNCTION<UserComponent4>(ARGS); break; \
            case 5: RET EE.COMPONENT_FUNCTION<UserComponent5>(ARGS); break; \
            case 6: RET EE.COMPONENT_FUNCTION<UserComponent6>(ARGS); break; \
            case 7: RET EE.COMPONENT_FUNCTION<UserComponent7>(ARGS); break; \
            case 8: RET EE.COMPONENT_FUNCTION<UserComponent8>(ARGS); break; \
            case 9: RET EE.COMPONENT_FUNCTION<UserComponent9>(ARGS); break; \
            case 10: RET EE.COMPONENT_FUNCTION<UserComponent10>(ARGS); break; \
            case 11: RET EE.COMPONENT_FUNCTION<UserComponent11>(ARGS); break; \
            case 12: RET EE.COMPONENT_FUNCTION<UserComponent12>(ARGS); break; \
            case 13: RET EE.COMPONENT_FUNCTION<UserComponent13>(ARGS); break; \
            case 14: RET EE.COMPONENT_FUNCTION<UserComponent14>(ARGS); break; \
            case 15: RET EE.COMPONENT_FUNCTION<UserComponent15>(ARGS); break; \
            default: assert(false); \
        }

    bool _hasComponent(entityx::Entity e, ComponentType t){
        #define COMPONENT_FUNCTION has_component
            COMPONENT_SWITCH(e, t, , return);
        #undef COMPONENT_FUNCTION
    }

    void UserComponentLogic::add(eId id, ComponentType t){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        if(_hasComponent(entity, t)) return;

        ComponentType type = 0;
        #define COMPONENT_FUNCTION assign
            COMPONENT_SWITCH(entity, type, , );
        #undef COMPONENT_FUNCTION
    }

    void UserComponentLogic::remove(eId id, ComponentType t){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        if(!_hasComponent(entity, t)) return;

        ComponentType type = 0;
        #define COMPONENT_FUNCTION remove
            COMPONENT_SWITCH(entity, type, , );
        #undef COMPONENT_FUNCTION
    }
}

