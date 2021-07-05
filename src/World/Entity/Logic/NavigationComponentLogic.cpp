#include "NavigationComponentLogic.h"

#include "World/Entity/EntityManager.h"
#include "World/Entity/Callback/EntityCallbackManager.h"
#include "World/Entity/Components/RigidBodyComponent.h"

#include "World/WorldSingleton.h"
#include "System/SystemSetup/SystemSettings.h"
#include "World/Nav/NavMeshManager.h"
#include "World/Entity/Components/NavigationComponent.h"
#include "DetourNavMeshQuery.h"

#include "World/Entity/Components/PositionComponent.h"

#include "entityx/entityx.h"
#include "Logger/Log.h"

namespace AV{
    bool NavigationComponentLogic::add(eId id){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        if(entity.has_component<RigidBodyComponent>()) return false;

        World* w = WorldSingleton::getWorld();
        if(!w) return false;

        entity.assign<NavigationComponent>();

        return true;
    }

    bool NavigationComponentLogic::remove(eId id){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        World* w = WorldSingleton::getWorld();
        if(!w) return false;

        entityx::ComponentHandle<NavigationComponent> compMesh = entity.component<NavigationComponent>();
        if(compMesh){
            NavQueryId queryId = compMesh.get()->queryId;
            if(queryId == INVALID_NAV_QUERY){
                //Nothing to destroy.
                return true;
            }
            std::shared_ptr<NavMeshManager> navMeshManager = w->getNavMeshManager();
            assert(navMeshManager);
            navMeshManager->releaseNavMeshQuery(queryId);

            entity.remove<NavigationComponent>();
            return true;
        }

        return false;
    }

    bool NavigationComponentLogic::navigateTo(eId id, const SlotPosition& pos, float targetSpeed){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        World* w = WorldSingleton::getWorld();
        if(!w) return false;

        entityx::ComponentHandle<NavigationComponent> compMesh = entity.component<NavigationComponent>();
        if(compMesh){
            NavQueryId queryId = compMesh.get()->queryId;
            std::shared_ptr<NavMeshManager> navMeshManager = w->getNavMeshManager();
            assert(navMeshManager);
            if(queryId == INVALID_NAV_QUERY){
                //Create a query.
                //TODO this needs to be a proper mesh value.
                queryId = navMeshManager->generateNavQuery(0);
                compMesh.get()->queryId = queryId;
            }

            //Find the path initially.
            const Ogre::Vector3 startVec = entity.component<PositionComponent>().get()->pos.toOgre();
            int result = navMeshManager->queryPath(queryId, startVec, pos.toOgre(), Ogre::Vector3(100, 100, 100));
            compMesh.get()->findingPath = result >= 0;
            compMesh.get()->targetSpeed = targetSpeed;

            return true;
        }

        return false;
    }

    void NavigationComponentLogic::updatePathFinding(eId id){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        entityx::ComponentHandle<NavigationComponent> compMesh = entity.component<NavigationComponent>();
        if(!compMesh) return;
        //Check if the path finding is actually in progress.
        if(!compMesh.get()->findingPath) return;

        //By this point the component should contain a work in progress path.
        World* w = WorldSingleton::getWorld();
        if(!w) return;

        std::shared_ptr<NavMeshManager> navMeshManager = w->getNavMeshManager();
        assert(navMeshManager);

        float targetSpeed = compMesh.get()->targetSpeed;
        const Ogre::Vector3 startVec = entity.component<PositionComponent>().get()->pos.toOgre();
        Ogre::Vector3 endVec;
        bool stillMoving = navMeshManager->getNextPosition(compMesh.get()->queryId, startVec, &endVec, targetSpeed);
        compMesh.get()->findingPath = stillMoving;
        if(!stillMoving) return;

        SlotPosition targetPos(endVec);
        AV_ERROR("Setting position {}", targetPos);
        w->getEntityManager()->setEntityPosition(id, targetPos);
    }
}
