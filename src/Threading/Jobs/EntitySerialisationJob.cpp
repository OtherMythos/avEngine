#include "EntitySerialisationJob.h"

#include "Entity/EntityManager.h"

#include "Entity/Components/PositionComponent.h"
#include "Entity/Logic/OgreMeshComponentLogic.h"
#include "Entity/Logic/ScriptComponentLogic.h"
#include "Entity/Logic/FundamentalLogic.h"

#include "Entity/Components/OgreMeshComponent.h"
#include "Entity/Components/ScriptComponent.h"

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
