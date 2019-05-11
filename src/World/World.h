#pragma once

#include "Logger/Log.h"
#include <memory>
#include <atomic>

namespace AV {
    class WorldSingleton;
    class SlotManager;
    class ChunkRadiusLoader;
    class EntityManager;
    class SaveHandle;
    class SerialiserStringStore;

    class Event;

    /**
     A class to encapsulate the functionality of the world.

     In itself the world is a collection of a number of pieces.
     This class serves as an encapsulation of all of these things.

     There should only be a single world in the engine at a time.
     This is managed by the WorldSingleton class, and the world shouldn't be created directly.
     */
    class World{
        friend WorldSingleton;
    protected:
        World();
        World(const SaveHandle& handle);
        ~World();
        
        void _initialise();
        
        /**
        Constructs the world from a serialised save.
        This will leave the world in an un-ready state until the serialisation job has finished.
        */
        void _deserialise(const SaveHandle& handle);
        
        void _finishDeSerialisation();
        
        SerialiserStringStore* mEntityMeshStore;
        SerialiserStringStore* mEntityScriptStore;

        std::atomic<int> serialisationJobCounter;

        std::shared_ptr<SlotManager> mSlotManager;
        std::shared_ptr<ChunkRadiusLoader> mChunkRadiusLoader;

        std::shared_ptr<EntityManager> mEntityManager;

    public:
        void update();

        void serialise(const SaveHandle& handle);

        std::shared_ptr<SlotManager> getSlotManager() { return mSlotManager; };
        std::shared_ptr<EntityManager> getEntityManager() { return mEntityManager; };
        std::shared_ptr<ChunkRadiusLoader> getChunkRadiusLoader() { return mChunkRadiusLoader; };
    };
}
