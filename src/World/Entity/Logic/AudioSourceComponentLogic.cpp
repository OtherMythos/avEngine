#include "AudioSourceComponentLogic.h"

#include "World/Entity/Components/AudioSourceComponent.h"
#include "entityx/entityx.h"

#include "Audio/AudioSource.h"

namespace AV{
    bool AudioSourceComponentLogic::add(eId id, AudioSourcePtr a, AudioSourcePtr b, bool aPopulated, bool bPopulated){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));
        if(entity.has_component<AudioSourceComponent>()) return false;

        entity.assign<AudioSourceComponent>(a, b, aPopulated, bPopulated);

        return true;
    }

    bool AudioSourceComponentLogic::remove(eId id){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));
        entityx::ComponentHandle<AudioSourceComponent> comp = entity.component<AudioSourceComponent>();
        if(!comp) return false;
        AudioSourceComponent *c = comp.get();

        c->objA.reset();
        c->objB.reset();
        entity.remove<AudioSourceComponent>();

        return true;
    }

    AudioSourcePtr AudioSourceComponentLogic::getAudioSource(eId id, uint8 animId){
        assert(animId < 2);
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        entityx::ComponentHandle<AudioSourceComponent> comp = entity.component<AudioSourceComponent>();
        if(!comp) return 0;
        AudioSourceComponent *c = comp.get();

        return animId == 0 ? c->objA : c->objB;
    }

    bool AudioSourceComponentLogic::setAudioSource(eId id, uint8 animId, AudioSourcePtr ptr){
        assert(animId < 2);
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        entityx::ComponentHandle<AudioSourceComponent> comp = entity.component<AudioSourceComponent>();
        if(!comp) return false;
        AudioSourceComponent *c = comp.get();

        if(animId == 0) c->objA = ptr;
        else c->objB = ptr;

        return true;
    }

    void AudioSourceComponentLogic::repositionKnown(eId id, const Ogre::Vector3& pos){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        entityx::ComponentHandle<AudioSourceComponent> audioComp = entity.component<AudioSourceComponent>();
        if(audioComp) {
            const AudioSourceComponent* data = audioComp.get();
            if(data->aPopulated) data->objA->setPosition(pos);
            if(data->bPopulated) data->objB->setPosition(pos);
        }
    }
}
