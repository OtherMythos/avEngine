#include "RigidBodyComponentLogic.h"

#include "World/Entity/EntityManager.h"
#include "World/Entity/Callback/EntityCallbackManager.h"
#include "World/Entity/Components/RigidBodyComponent.h"

#include "World/WorldSingleton.h"
#include "World/Physics/PhysicsManager.h"
#include "World/Physics/Worlds/DynamicsWorld.h"

#include "entityx/entityx.h"

namespace AV{
    bool RigidBodyComponentLogic::add(eId id, PhysicsBodyConstructor::RigidBodyPtr body){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        if(entity.has_component<RigidBodyComponent>()) return false;

        World* w = WorldSingleton::getWorld();
        if(!w) return false;

        w->getPhysicsManager()->getDynamicsWorld()->attachEntityToBody(body, id);

        entity.assign<RigidBodyComponent>(body);

        return true;
    }

    bool RigidBodyComponentLogic::remove(eId id){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));
        if(!entity.has_component<RigidBodyComponent>()) return false;

        World* w = WorldSingleton::getWorld();
        if(!w) return false;

        entityx::ComponentHandle<RigidBodyComponent> comp = entity.component<RigidBodyComponent>();
        w->getPhysicsManager()->getDynamicsWorld()->detatchEntityFromBody(comp.get()->body);

        entity.remove<RigidBodyComponent>();

        return true;
    }

    bool RigidBodyComponentLogic::getBody(eId id, PhysicsBodyConstructor::RigidBodyPtr& body){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        //Something invalid, maybe a shared pointer.
        if(!entity.has_component<RigidBodyComponent>()) return false;

        entityx::ComponentHandle<RigidBodyComponent> comp = entity.component<RigidBodyComponent>();
        body = comp.get()->body;
        return true;
    }

    void RigidBodyComponentLogic::serialise(std::ofstream& stream, entityx::Entity& e){
        entityx::ComponentHandle<RigidBodyComponent> comp = e.component<RigidBodyComponent>();

    }

    void RigidBodyComponentLogic::deserialise(eId entity, std::ifstream& file, SerialiserStringStore *store){

    }
}
