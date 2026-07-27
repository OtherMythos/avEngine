#include "ScriptComponentLogic.h"

#include "Entity/EntityManager.h"
#include "Entity/Callback/EntityCallbackManager.h"
#include "Entity/Components/ScriptComponent.h"


#include "entityx/entityx.h"
#include <fstream>

namespace AV{
    ScriptComponentAddResult ScriptComponentLogic::add(eId id, const Ogre::String &scriptPath){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        if(entity.has_component<ScriptComponent>()) return ScriptComponentAddResult::ALREADY_HAS_COMPONENT;

        int scriptId = entityManager->getEntityCallbackManager()->loadScript(scriptPath);
        if(scriptId < 0){
            return ScriptComponentAddResult::ALREADY_HAS_COMPONENT;
        }

        bool hasUpdate = false;
        entityManager->getEntityCallbackManager()->getMetadataOfScript(scriptId, hasUpdate);

        entity.assign<ScriptComponent>(scriptId, hasUpdate);

        return ScriptComponentAddResult::SUCCESS;
    }

    bool ScriptComponentLogic::remove(eId id){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));
        if(!entity.has_component<ScriptComponent>()) return false;

        entityx::ComponentHandle<ScriptComponent> comp = entity.component<ScriptComponent>();
        int script = comp.get()->scriptId;

        entityManager->getEntityCallbackManager()->unreferenceScript(script);

        entity.remove<ScriptComponent>();

        return false;
    }

}
