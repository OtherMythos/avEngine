#include "RigidBodyComponentLogic.h"

#include "World/Entity/EntityManager.h"
#include "World/Entity/Callback/EntityCallbackManager.h"
#include "World/Entity/Components/RigidBodyComponent.h"

#include "entityx/entityx.h"

namespace AV{
    void RigidBodyComponentLogic::add(eId id, DynamicsWorld::RigidBodyPtr body){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        if(entity.has_component<RigidBodyComponent>()) return;

        entity.assign<RigidBodyComponent>(body);
    }

    bool RigidBodyComponentLogic::remove(eId id){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));
        if(!entity.has_component<RigidBodyComponent>()) return false;

        entity.remove<RigidBodyComponent>();

        return false;
    }

    void RigidBodyComponentLogic::serialise(std::ofstream& stream, entityx::Entity& e){
        entityx::ComponentHandle<RigidBodyComponent> comp = e.component<RigidBodyComponent>();

    }

    void RigidBodyComponentLogic::deserialise(eId entity, std::ifstream& file, SerialiserStringStore *store){

    }
}
