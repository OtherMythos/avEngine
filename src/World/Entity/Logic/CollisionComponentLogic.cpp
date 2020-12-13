#include "CollisionComponentLogic.h"

#include "World/Entity/EntityManager.h"
#include "World/Entity/Callback/EntityCallbackManager.h"
#include "World/Entity/Components/CollisionComponent.h"

#include "World/WorldSingleton.h"
#include "World/Physics/PhysicsManager.h"
#include "World/Physics/Worlds/CollisionWorld.h"

#include "entityx/entityx.h"

namespace AV{
    bool CollisionComponentLogic::add(eId id, PhysicsTypes::CollisionObjectPtr a, PhysicsTypes::CollisionObjectPtr b, bool aPopulated, bool bPopulated){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        if(entity.has_component<CollisionComponent>()) return false;

        if(aPopulated) CollisionWorld::attachObjectToEntity(a, id);
        if(bPopulated) CollisionWorld::attachObjectToEntity(b, id);
        entity.assign<CollisionComponent>(a, b, aPopulated, bPopulated);

        return true;
    }

    bool CollisionComponentLogic::remove(eId id){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));
        entityx::ComponentHandle<CollisionComponent> comp = entity.component<CollisionComponent>();
        if(!comp) return false;
        CollisionComponent *c = comp.get();
        if(c->aPopulated) CollisionWorld::detachObjectFromEntity(c->objA);
        if(c->bPopulated) CollisionWorld::detachObjectFromEntity(c->objB);

        entity.remove<CollisionComponent>();

        return true;
    }

    bool CollisionComponentLogic::getBody(eId id, bool a, PhysicsTypes::CollisionObjectPtr* outObj){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        //Something invalid, maybe a shared pointer.
        if(!entity.has_component<CollisionComponent>()) return false;

        entityx::ComponentHandle<CollisionComponent> comp = entity.component<CollisionComponent>();
        *outObj = a ? comp.get()->objA : comp.get()->objA;
        return true;
    }

    void CollisionComponentLogic::repositionKnown(eId id, const Ogre::Vector3& pos){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        entityx::ComponentHandle<CollisionComponent> collisionComp = entity.component<CollisionComponent>();
        if(collisionComp) {
            const CollisionComponent* data = collisionComp.get();
            if(data->aPopulated) CollisionWorld::setObjectPositionStatic(data->objA, OGRE_TO_BULLET(pos));
            if(data->bPopulated) CollisionWorld::setObjectPositionStatic(data->objB, OGRE_TO_BULLET(pos));
        }
    }
}
