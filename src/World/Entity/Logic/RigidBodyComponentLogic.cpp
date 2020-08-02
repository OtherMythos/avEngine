#include "RigidBodyComponentLogic.h"

#include "World/Entity/EntityManager.h"
#include "World/Entity/Callback/EntityCallbackManager.h"
#include "World/Entity/Components/RigidBodyComponent.h"

#include "World/WorldSingleton.h"
#include "World/Physics/PhysicsManager.h"
#include "World/Physics/Worlds/DynamicsWorld.h"
#include "System/SystemSetup/SystemSettings.h"

#include "entityx/entityx.h"

namespace AV{
    bool RigidBodyComponentLogic::add(eId id, PhysicsTypes::RigidBodyPtr body){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        if(entity.has_component<RigidBodyComponent>()) return false;

        World* w = WorldSingleton::getWorld();
        if(!w) return false;

        std::shared_ptr<DynamicsWorld> dynWorld = w->getPhysicsManager()->getDynamicsWorld();
        if(!dynWorld) return false;
        if(!dynWorld->attachEntityToBody(body, id)) return false;

        entity.assign<RigidBodyComponent>(body);

        return true;
    }

    bool RigidBodyComponentLogic::remove(eId id){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));
        if(!entity.has_component<RigidBodyComponent>()) return false;

        World* w = WorldSingleton::getWorld();
        if(!w) return false;

        std::shared_ptr<DynamicsWorld> dynWorld = w->getPhysicsManager()->getDynamicsWorld();
        //If the component was created, then the dynWorld should still be there when we come to destroy it.
        assert(dynWorld);
        entityx::ComponentHandle<RigidBodyComponent> comp = entity.component<RigidBodyComponent>();
        dynWorld->detatchEntityFromBody(comp.get()->body);

        entity.remove<RigidBodyComponent>();

        return true;
    }

    bool RigidBodyComponentLogic::getBody(eId id, PhysicsTypes::RigidBodyPtr& body){
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
