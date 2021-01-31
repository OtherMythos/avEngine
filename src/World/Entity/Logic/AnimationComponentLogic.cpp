#include "AnimationComponentLogic.h"

#include "World/Entity/Components/AnimationComponent.h"
#include "entityx/entityx.h"

namespace AV{
    bool AnimationComponentLogic::add(eId id, SequenceAnimationPtr a, SequenceAnimationPtr b, bool aPopulated, bool bPopulated){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));
        if(entity.has_component<AnimationComponent>()) return false;

        entity.assign<AnimationComponent>(a, b, aPopulated, bPopulated);

        return true;
    }

    bool AnimationComponentLogic::remove(eId id){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));
        entityx::ComponentHandle<AnimationComponent> comp = entity.component<AnimationComponent>();
        if(!comp) return false;
        AnimationComponent *c = comp.get();

        c->objA.reset();
        c->objB.reset();
        entity.remove<AnimationComponent>();

        return true;
    }

    SequenceAnimationPtr AnimationComponentLogic::getAnimation(eId id, uint8 animId){
        assert(animId < 2);
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        entityx::ComponentHandle<AnimationComponent> comp = entity.component<AnimationComponent>();
        if(!comp) return 0;
        AnimationComponent *c = comp.get();

        return animId == 0 ? c->objA : c->objB;
    }
}
