#include "UserComponentLogic.h"

#include "entityx/entityx.h"

namespace AV{

    struct UserComponent{
        UserComponent(ComponentId id) : mId(id) { }
        ComponentId mId;
    };

    struct UserComponent0 : public UserComponent{ UserComponent0(ComponentId id) : UserComponent(id) { } };
    struct UserComponent1 : public UserComponent{ UserComponent1(ComponentId id) : UserComponent(id) { } };
    struct UserComponent2 : public UserComponent{ UserComponent2(ComponentId id) : UserComponent(id) { } };
    struct UserComponent3 : public UserComponent{ UserComponent3(ComponentId id) : UserComponent(id) { } };
    struct UserComponent4 : public UserComponent{ UserComponent4(ComponentId id) : UserComponent(id) { } };
    struct UserComponent5 : public UserComponent{ UserComponent5(ComponentId id) : UserComponent(id) { } };
    struct UserComponent6 : public UserComponent{ UserComponent6(ComponentId id) : UserComponent(id) { } };
    struct UserComponent7 : public UserComponent{ UserComponent7(ComponentId id) : UserComponent(id) { } };
    struct UserComponent8 : public UserComponent{ UserComponent8(ComponentId id) : UserComponent(id) { } };
    struct UserComponent9 : public UserComponent{ UserComponent9(ComponentId id) : UserComponent(id) { } };
    struct UserComponent10 : public UserComponent{ UserComponent10(ComponentId id) : UserComponent(id) { } };
    struct UserComponent11 : public UserComponent{ UserComponent11(ComponentId id) : UserComponent(id) { } };
    struct UserComponent12 : public UserComponent{ UserComponent12(ComponentId id) : UserComponent(id) { } };
    struct UserComponent13 : public UserComponent{ UserComponent13(ComponentId id) : UserComponent(id) { } };
    struct UserComponent14 : public UserComponent{ UserComponent14(ComponentId id) : UserComponent(id) { } };
    struct UserComponent15 : public UserComponent{ UserComponent15(ComponentId id) : UserComponent(id) { } };

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

    void UserComponentLogic::add(eId id, ComponentType t, ComponentId comp){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        if(_hasComponent(entity, t)) return;

        #define COMPONENT_FUNCTION assign
            COMPONENT_SWITCH(entity, t, comp, );
        #undef COMPONENT_FUNCTION
    }

    void UserComponentLogic::remove(eId id, ComponentType t){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        if(!_hasComponent(entity, t)) return;

        #define COMPONENT_FUNCTION remove
            COMPONENT_SWITCH(entity, t, , );
        #undef COMPONENT_FUNCTION
    }
}

