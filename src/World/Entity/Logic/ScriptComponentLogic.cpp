#include "ScriptComponentLogic.h"

#include "World/Entity/EntityManager.h"
#include "World/Entity/Callback/EntityCallbackManager.h"
#include "World/Entity/Components/ScriptComponent.h"

#include "Serialisation/SerialiserStringStore.h"

#include "entityx/entityx.h"

namespace AV{
    void ScriptComponentLogic::add(eId id, const Ogre::String &scriptPath){
        entityx::Entity entity(&(entityXManager->entities), entityx::Entity::Id(id.id()));

        if(entity.has_component<ScriptComponent>()) return;

        //TODO what if this fails to load a valid script?
        int scriptId = entityManager->getEntityCallbackManager()->loadScript(scriptPath);

        bool hasUpdate = false;
        entityManager->getEntityCallbackManager()->getMetadataOfScript(scriptId, hasUpdate);

        entity.assign<ScriptComponent>(scriptId, hasUpdate);
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

    void ScriptComponentLogic::serialise(std::ofstream& stream, entityx::Entity& e){
        entityx::ComponentHandle<ScriptComponent> comp = e.component<ScriptComponent>();

        stream << "[Script]\n";
        //TODO this is just a note for future.
        //This actually returns the entire path to the script, which might be specific to the user's device.
        //In that case the saves wouldn't be transferrable between devices, so this should be addressed.
        //However, it can't really be addressed until I need to implement a better solution to the script resource access.
        stream << entityManager->getEntityCallbackManager()->getScriptPath(comp.get()->scriptId) << std::endl;
    }

    void ScriptComponentLogic::deserialise(eId entity, std::ifstream& file, SerialiserStringStore *store){
        std::string line;
        getline(file, line);

        store->mStoredStrings.push_back({entity, line});
    }
}
