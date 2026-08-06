#include "EntityManager.h"


#include "Physics/PhysicsManager.h"
#include "Physics/Worlds/DynamicsWorld.h"

#include "Components/PositionComponent.h"
#include "Components/OgreMeshComponent.h"
#include "Components/SceneNodeComponent.h"
#include "Components/ScriptComponent.h"
#include "Components/RigidBodyComponent.h"
#include "Components/CollisionComponent.h"
#include "Components/LifetimeComponent.h"
#include "Components/SceneNodeComponent.h"
#include "Components/AudioSourceComponent.h"

#include "Logic/ComponentLogic.h"
#include "Logic/OgreMeshComponentLogic.h"
#include "Logic/SceneNodeComponentLogic.h"
#include "Logic/ScriptComponentLogic.h"
#include "Logic/CollisionComponentLogic.h"
#include "Logic/UserComponentLogic.h"
#include "Logic/AudioSourceComponentLogic.h"


#include "Event/EventDispatcher.h"
#include "System/SystemSetup/SystemSettings.h"

#include "Entity/UserComponents/UserComponentManager.h"

#include "Callback/EntityCallbackManager.h"
#include "System/Util/OgreNodeHelper.h"
#include "System/Pause/PauseState.h"

#include "Logger/Log.h"

namespace AV{
    EntityManager::EntityManager()
        : mUserComponentManager(std::make_shared<UserComponentManager>()){

    }

    EntityManager::~EntityManager(){
        AV_INFO("Shutting down the Entity Manager.");


        ComponentLogic::entityManager = 0;
        ComponentLogic::entityXManager = 0;
    }

    void EntityManager::update(){
        if(!SystemSettings::getDynamicPhysicsDisabled()){
            const std::vector<DynamicsWorld::EntityTransformData>& data = mPhysicsManager->getDynamicsWorld()->getEntityTransformData();
            for(const DynamicsWorld::EntityTransformData& e : data){
                Ogre::Vector3 pos(e.pos.x(), e.pos.y(), e.pos.z());
                //The true flag will make sure that the rigid body is not updated by this move.
                setEntityPosition(e.entity, Ogre::Vector3(pos), true);
            }
        }

        if((PauseState::getMask() & PAUSE_TYPE_LIFETIME_COMPONENT) == 0){
            ex.entities.each<LifetimeComponent>([this](entityx::Entity entity, LifetimeComponent &life){
                //TODO remove
                uint64 deltaTime = 1;
                if(deltaTime >= life.remainingTime){
                    this->destroyEntity(_eId(entity));
                }
                life.remainingTime -= deltaTime;
            });
        }

        //Call the routine functions.
        if((PauseState::getMask() & PAUSE_TYPE_ENTITY_UPDATE) == 0){
            ex.entities.each<ScriptComponent>([this](entityx::Entity entity, ScriptComponent &s){
                if(s.scriptHasUpdate){
                    this->notifyEntityEvent(_eId(entity), EntityEventType::UPDATE);
                }
            });
        }
    }

    void EntityManager::initialise(){
        //I can safely assume here that if an entity manager is being constructed that means a new world is being constructed.
        //So here is the best place to inject the pointer value into the component logic.
        ComponentLogic::entityManager = this;
        ComponentLogic::entityXManager = &ex;

        mEntityCallbackManager = std::make_shared<EntityCallbackManager>();

    }

    entityx::Entity EntityManager::_createEntity(Ogre::Vector3 pos){
        AV_INFO("Creating entity at position {}", pos);
        entityx::Entity entity = ex.entities.create();

        entity.assign<PositionComponent>(pos);

        return entity;
    }

    eId EntityManager::createEntity(Ogre::Vector3 pos){
        return _eId(_createEntity(pos));
    }

    void EntityManager::destroyKnownEntity(eId entity){
        AV_INFO("Destroying entity {}", entity.id());

        //Send the event first, so that all the entity state is before the destruction.
        notifyEntityEvent(entity, EntityEventType::DESTROYED);

        entityx::Entity e = getEntityHandle(entity);
        entityx::ComponentHandle<OgreMeshComponent> meshComponent = e.component<OgreMeshComponent>();
        if(meshComponent) OgreMeshComponentLogic::remove(entity);

        entityx::ComponentHandle<ScriptComponent> scriptComponent = e.component<ScriptComponent>();
        if(scriptComponent) ScriptComponentLogic::remove(entity);

        entityx::ComponentHandle<SceneNodeComponent> nodeComponent = e.component<SceneNodeComponent>();
        if(nodeComponent){
            SceneNodeComponent* comp = nodeComponent.get();
            if(comp->destroyNodeOnDestruction){
                OgreNodeHelper::destroyNodeAndChildren(comp->node);
            }
        }

        UserComponentLogic::removeEntity(entity);

        e.destroy();
    }

    void EntityManager::destroyEntity(eId entity){
        if(entity == eId::INVALID) return;
        destroyKnownEntity(entity);
    }

    void EntityManager::setEntityOrientation(eId id, Ogre::Quaternion orientation){
        OgreMeshComponentLogic::orientate(id, orientation);
    }

    void EntityManager::setEntityPosition(eId id, Ogre::Vector3 position, bool autoMove){
        entityx::Entity e = getEntityHandle(id);
        if(!e.valid()) return;

        entityx::ComponentHandle<PositionComponent> compPos = e.component<PositionComponent>();

        if(compPos){
            compPos.get()->pos = position;
        }

        Ogre::Vector3 absPos = position;
        if(e.has_component<OgreMeshComponent>()){
            OgreMeshComponentLogic::repositionKnown(id, absPos);
        }
        if(e.has_component<SceneNodeComponent>()){
            SceneNodeComponentLogic::repositionKnown(id, absPos);
        }
        if(!autoMove){
            entityx::ComponentHandle<RigidBodyComponent> rigidBody = e.component<RigidBodyComponent>();
            if(rigidBody){
                //If the entity has a rigid body component then physics should exist.
                assert(!SystemSettings::getDynamicPhysicsDisabled());
                btVector3 btAbsPos(absPos.x, absPos.y, absPos.z);
                mPhysicsManager->getDynamicsWorld()->setBodyPosition(rigidBody.get()->body, btAbsPos);
            }
        }
        if(e.has_component<CollisionComponent>()){
            CollisionComponentLogic::repositionKnown(id, absPos);
        }
        if(e.has_component<AudioSourceComponent>()){
            AudioSourceComponentLogic::repositionKnown(id, absPos);
        }

        notifyEntityEvent(id, EntityEventType::MOVED);
    }

    void EntityManager::notifyEntityEvent(eId entity, EntityEventType event){
        entityx::Entity e = getEntityHandle(entity);
        if(!e.valid()) return;

        entityx::ComponentHandle<ScriptComponent> comp = e.component<ScriptComponent>();
        if(comp){
            mEntityCallbackManager->notifyEvent(entity, event, comp.get()->scriptId);
        }
    }

    bool EntityManager::getEntityValid(eId entity){
        entityx::Entity e = getEntityHandle(entity);
        if(e.valid()) return true;

        return false;
    }

    void EntityManager::getDebugInfo(EntityDebugInfo *info){
        info->totalEntities = static_cast<int>(ex.entities.size());
        info->totalCallbackScripts = mEntityCallbackManager->getActiveScripts();
    }

}
