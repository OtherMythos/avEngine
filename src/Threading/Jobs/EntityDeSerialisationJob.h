#pragma once

#include <iostream>
#include "Job.h"
#include "World/Slot/Recipe/OgreMeshRecipeData.h"
#include "entityx/entityx.h"
#include "Serialisation/SaveHandle.h"
#include <atomic>
#include <memory>

namespace AV{
    class SaveHandle;
    class EntityManager;
    class eId;
    class SerialiserStringStore;

    class EntityDeSerialisationJob : public Job{
    public:
        EntityDeSerialisationJob(const SaveHandle& handle, std::atomic<int> *progressCounter, std::shared_ptr<EntityManager> manager, SerialiserStringStore *entityMeshStore, SerialiserStringStore *entityScriptStore);

        void process();
        void finish();

    private:
        std::atomic<int> *mProgressCounter;
        std::shared_ptr<EntityManager> mEntityManager;
        const SaveHandle mSaveHandle;
        
        void _beginEntity(std::ifstream& file);
        void _iterateEntityComponents(eId entity, std::ifstream& file, const std::string &line);
        bool _processPositionComponent(std::ifstream& file, eId& entity);
        
        SerialiserStringStore* mEntityMeshStore;
        SerialiserStringStore* mEntityScriptStore;
        
        //Move the current line cursor on the file to the next entity separator.
        void _abortEntity(std::ifstream& file);

        //void _serialiseEntity(std::ofstream& stream, entityx::Entity entity);
    };
};
