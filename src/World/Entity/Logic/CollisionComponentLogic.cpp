#include "CollisionComponentLogic.h"

#include "World/Entity/EntityManager.h"
#include "World/Entity/Callback/EntityCallbackManager.h"
#include "World/Entity/Components/CollisionComponent.h"

#include "World/WorldSingleton.h"
#include "World/Physics/PhysicsManager.h"
#include "World/Physics/Worlds/CollisionWorld.h"

#include "entityx/entityx.h"

namespace AV{
    bool CollisionComponentLogic::add(eId id, PhysicsTypes::CollisionObjectPtr a, PhysicsTypes::CollisionObjectPtr b, PhysicsTypes::CollisionObjectPtr c, bool aPopulated, bool bPopulated, bool cPopulated){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        if(entity.has_component<CollisionComponent>()) return false;

        if(aPopulated) CollisionWorld::attachObjectToEntity(a, id);
        if(bPopulated) CollisionWorld::attachObjectToEntity(b, id);
        if(cPopulated) CollisionWorld::attachObjectToEntity(c, id);
        entity.assign<CollisionComponent>(a, b, c, aPopulated, bPopulated, cPopulated);

        return true;
    }

    bool CollisionComponentLogic::remove(eId id){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));
        entityx::ComponentHandle<CollisionComponent> comp = entity.component<CollisionComponent>();
        if(!comp) return false;
        CollisionComponent *c = comp.get();
        if(c->aPopulated) CollisionWorld::detachObjectFromEntity(c->objA);
        if(c->bPopulated) CollisionWorld::detachObjectFromEntity(c->objB);
        if(c->cPopulated) CollisionWorld::detachObjectFromEntity(c->objC);

        entity.remove<CollisionComponent>();

        return true;
    }

    bool CollisionComponentLogic::getBody(eId id, uint8 body, PhysicsTypes::CollisionObjectPtr* outObj){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        //Something invalid, maybe a shared pointer.
        if(!entity.has_component<CollisionComponent>()) return false;

        entityx::ComponentHandle<CollisionComponent> comp = entity.component<CollisionComponent>();
        switch(body){
            case 0: *outObj = comp.get()->objA; break;
            case 1: *outObj = comp.get()->objB; break;
            case 2: *outObj = comp.get()->objC; break;
        }
        return true;
    }

    void CollisionComponentLogic::repositionKnown(eId id, const Ogre::Vector3& pos){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        entityx::ComponentHandle<CollisionComponent> collisionComp = entity.component<CollisionComponent>();
        if(collisionComp) {
            const CollisionComponent* data = collisionComp.get();
            btVector3 bulletPos = OGRE_TO_BULLET(pos);
            if(data->aPopulated) CollisionWorld::setObjectPositionStatic(data->objA, bulletPos);
            if(data->bPopulated) CollisionWorld::setObjectPositionStatic(data->objB, bulletPos);
            if(data->cPopulated) CollisionWorld::setObjectPositionStatic(data->objC, bulletPos);
        }
    }
}
