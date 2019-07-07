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
    class MeshSerialisationBuilder;

    class EntitySerialisationJob : public Job{
    public:
        EntitySerialisationJob(const SaveHandle& handle, std::atomic<int> *progressCounter, std::shared_ptr<EntityManager> manager, std::shared_ptr<MeshSerialisationBuilder> meshSerialisationBuilder);

        void process();
        void finish();

    private:
        std::atomic<int> *mProgressCounter;
        std::shared_ptr<EntityManager> mEntityManager;
        std::shared_ptr<MeshSerialisationBuilder> mMeshSerialisationBuilder;
        const SaveHandle mSaveHandle;

        void _serialiseEntity(std::ofstream& stream, entityx::Entity entity);
    };
};
