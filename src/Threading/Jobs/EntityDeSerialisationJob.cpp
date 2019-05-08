#include "EntityDeSerialisationJob.h"

#include "World/Entity/EntityManager.h"

#include "World/Entity/Components/PositionComponent.h"
#include "World/Entity/Logic/OgreMeshComponentLogic.h"
#include "World/Entity/Logic/ScriptComponentLogic.h"
#include "World/Entity/Logic/FundamentalLogic.h"

#include "World/Entity/Components/OgreMeshComponent.h"
#include "World/Entity/Components/ScriptComponent.h"

#include "Logger/Log.h"

namespace AV{
    EntityDeSerialisationJob::EntityDeSerialisationJob(const SaveHandle& handle, std::atomic<int> *progressCounter, std::shared_ptr<EntityManager> manager)
        : mProgressCounter(progressCounter),
          mEntityManager(manager),
          mSaveHandle(handle){

    }

    void EntityDeSerialisationJob::process(){
        AV_INFO("Starting world entity deSerialisation job!");

        std::string line;
        std::ifstream file(mSaveHandle.determineEntitySerialisedFile());
        if (file.is_open()){
            while(getline(file, line)){
                AV_INFO(line);
            }
        }
        
        file.close();
    }

    void EntityDeSerialisationJob::finish(){
        AV_INFO("Finishing world entity deSerialisation job!");

        (*mProgressCounter)++;
    }

}
