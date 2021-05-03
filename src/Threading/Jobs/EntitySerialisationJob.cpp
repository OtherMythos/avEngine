#include "EntitySerialisationJob.h"

#include "World/Entity/EntityManager.h"

#include "World/Entity/Components/PositionComponent.h"
#include "World/Entity/Logic/OgreMeshComponentLogic.h"
#include "World/Entity/Logic/ScriptComponentLogic.h"
#include "World/Entity/Logic/FundamentalLogic.h"

#include "World/Entity/Components/OgreMeshComponent.h"
#include "World/Entity/Components/ScriptComponent.h"

#include "Logger/Log.h"
#include <fstream>

namespace AV{
    EntitySerialisationJob::EntitySerialisationJob(const SaveHandle& handle, std::atomic<int> *progressCounter, std::shared_ptr<EntityManager> manager, std::shared_ptr<MeshSerialisationBuilder> meshSerialisationBuilder)
        : mProgressCounter(progressCounter),
          mEntityManager(manager),
          mSaveHandle(handle),
          mMeshSerialisationBuilder(meshSerialisationBuilder) {

    }

    void EntitySerialisationJob::process(){
        AV_INFO("Starting world entity serialisation job!");

        std::ofstream file;
        file.open(mSaveHandle.determineEntitySerialisedFile());

        //All entities have a position component, so this can be used as an iterator for all entities.
        mEntityManager->ex.entities.each<PositionComponent>([this, &file](entityx::Entity entity, PositionComponent &comp){
            _serialiseEntity(file, entity);
        });

        file.close();
    }

    void EntitySerialisationJob::finish(){
        AV_INFO("Finishing world entity serialisation job!");

        (*mProgressCounter)++;
    }

    void EntitySerialisationJob::_serialiseEntity(std::ofstream& stream, entityx::Entity entity){
        //All entities should have a position component.
        FundamentalLogic::serialise(stream, entity);

        if(entity.has_component<OgreMeshComponent>()) OgreMeshComponentLogic::serialise(stream, entity, mMeshSerialisationBuilder.get());
        if(entity.has_component<ScriptComponent>()) ScriptComponentLogic::serialise(stream, entity);

        stream << "--" << std::endl;
    }
}
