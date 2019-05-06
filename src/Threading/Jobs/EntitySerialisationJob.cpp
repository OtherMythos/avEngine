#include "EntitySerialisationJob.h"

#include "Serialisation/SaveHandle.h"
#include "World/Entity/EntityManager.h"

#include "World/Entity/Components/PositionComponent.h"

#include "Logger/Log.h"

namespace AV{
    EntitySerialisationJob::EntitySerialisationJob(const SaveHandle& handle, std::atomic<int> *progressCounter, std::shared_ptr<EntityManager> manager)
        : mProgressCounter(progressCounter),
          mEntityManager(manager){

    }

    void EntitySerialisationJob::process(){
        AV_INFO("Starting world entity serialisation job!");

        //All entities have a position component, so this can be used as an iterator for all entities.
        mEntityManager->ex.entities.each<PositionComponent>([](entityx::Entity entity, PositionComponent &comp){
            
        });
    }

    void EntitySerialisationJob::finish(){
        AV_INFO("Finishing world entity serialisation job!");

        (*mProgressCounter)++;
    }
}
