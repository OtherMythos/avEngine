#include "NavigationComponentLogic.h"

#include "World/Entity/EntityManager.h"
#include "World/Entity/Callback/EntityCallbackManager.h"
#include "World/Entity/Components/RigidBodyComponent.h"

#include "World/WorldSingleton.h"
#include "System/SystemSetup/SystemSettings.h"
#include "World/Nav/NavMeshManager.h"
#include "World/Entity/Components/NavigationComponent.h"

#include "entityx/entityx.h"

namespace AV{
    bool NavigationComponentLogic::add(eId id){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        if(entity.has_component<RigidBodyComponent>()) return false;

        World* w = WorldSingleton::getWorld();
        if(!w) return false;

        std::shared_ptr<NavMeshManager> navMeshManager = w->getNavMeshManager();
        assert(navMeshManager);
        //TODO this needs to be a proper mesh value.
        NavQueryId queryId = navMeshManager->generateNavQuery(0);

        entity.assign<NavigationComponent>(queryId);

        return true;
    }

    bool NavigationComponentLogic::remove(eId id){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        World* w = WorldSingleton::getWorld();
        if(!w) return false;

        entityx::ComponentHandle<NavigationComponent> compMesh = entity.component<NavigationComponent>();
        if(compMesh){
            NavQueryId queryId = compMesh.get()->queryId;
            std::shared_ptr<NavMeshManager> navMeshManager = w->getNavMeshManager();
            assert(navMeshManager);
            navMeshManager->releaseNavMeshQuery(queryId);

            entity.remove<NavigationComponent>();
            return true;
        }

        return false;
    }
}
