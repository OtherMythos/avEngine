#include "ScriptComponentLogic.h"

#include "World/Entity/EntityManager.h"
#include "World/Entity/Callback/EntityCallbackManager.h"
#include "World/Entity/Components/ScriptComponent.h"

#include "entityx/entityx.h"

namespace AV{
    void ScriptComponentLogic::add(eId id, const Ogre::String &scriptPath){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));
        
        if(entity.has_component<ScriptComponent>()) return;
        
        int scriptId = entityManager->getEntityCallbackManager()->loadScript(scriptPath);
        
        entity.assign<ScriptComponent>(scriptId);
    }
    
    bool ScriptComponentLogic::remove(eId id){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));
        if(!entity.has_component<ScriptComponent>()) return false;
        
        entityx::ComponentHandle<ScriptComponent> comp = entity.component<ScriptComponent>();
        int script = comp.get()->scriptId;
        
        entityManager->getEntityCallbackManager()->unreferenceScript(script);
        
        return false;
    }
}
