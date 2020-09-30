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

    bool NavigationComponentLogic::navigateTo(eId id, const SlotPosition& pos){
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

            //Call the navigation functions.
            dtNavMeshQuery* query = navMeshManager->getQuery(queryId);
            //Find the start and end pos.
            const Ogre::Vector3 endVec = pos.toOgre();
            const Ogre::Vector3 startVec = entity.component<PositionComponent>().get()->pos.toOgre();
            const float startPos[3] = {startVec.x, startVec.y, startVec.z};
            const float endPos[3] = {endVec.x, endVec.y, endVec.z};
            float startPolyPoint[3];
            float endPolyPoint[3];
            dtPolyRef startRef;
            dtPolyRef endRef;

            static const float extent[3] = {10.0f, 10.0f, 10.0f};
            dtQueryFilter filter;

            dtStatus result1 = query->findNearestPoly(startPos, extent, &filter, &startRef, &(startPolyPoint[0]));
            dtStatus result2 = query->findNearestPoly(endPos, extent, &filter, &endRef, &(endPolyPoint[0]));

            static const int MAX_POLYS = 256;
            dtPolyRef outPath[MAX_POLYS];
            int pathCount = 0;
            query->findPath(startRef, endRef, &(startPolyPoint[0]), &(endPolyPoint[0]), &filter, outPath, &pathCount, MAX_POLYS);

            if(pathCount){
                //A valid path was found.
            }

            return true;
        }

        return false;
    }
}
