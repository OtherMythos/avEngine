#include "World.h"

#include "Slot/SlotPosition.h"
#include "Slot/SlotManager.h"
#include "Slot/Chunk/ChunkFactory.h"
#include "Slot/Chunk/TerrainManager.h"
#include "Slot/ChunkCoordinate.h"
#include "Slot/ChunkRadiusLoader.h"

#include "Input/Input.h"

#include "OgreCamera.h"

#include "Event/EventDispatcher.h"
#include "Event/Events/WorldEvent.h"
#include "World/WorldSingleton.h"

#include "Serialisation/SerialiserStringStore.h"
#include "World/Entity/Logic/OgreMeshComponentLogic.h"
#include "World/Entity/Logic/ScriptComponentLogic.h"

#include "Threading/JobDispatcher.h"
#include "Threading/Jobs/EntitySerialisationJob.h"
#include "Threading/Jobs/EntityDeSerialisationJob.h"

#include "System/BaseSingleton.h"
#include "Serialisation/SerialisationManager.h"
#include "Serialisation/MeshSerialisationBuilder.h"

#include "Physics/PhysicsManager.h"

#include "Entity/EntityManager.h"

namespace AV {
    World::World()
        : mCreatedFromSave(false){
        _initialise();
    }

    World::World(const SaveHandle& handle)
        : mCreatedFromSave(true),
          mCreatorSaveHandle(handle){
        _initialise();
        _deserialise(handle);
    }

    void World::_initialise(){
        mEntityManager = std::make_shared<EntityManager>();
        mEntityManager->initialise();

        mPhysicsManager = std::make_shared<PhysicsManager>();

        std::shared_ptr<TerrainManager> terrMan = std::make_shared<TerrainManager>();
        std::shared_ptr<ChunkFactory> chunkFactory = std::make_shared<ChunkFactory>(mPhysicsManager, terrMan);
        chunkFactory->initialise();
        mSlotManager = std::make_shared<SlotManager>(chunkFactory);

        mChunkRadiusLoader = std::make_shared<ChunkRadiusLoader>(mSlotManager);

        WorldSingleton::setPlayerPosition(SlotPosition());

        //Setup necessary pointers.
        mEntityManager->setPhysicsManager(mPhysicsManager);

        mMeshSerialisationBuilder = std::make_shared<MeshSerialisationBuilder>(BaseSingleton::getOgreMeshManager());
    }

    void World::serialise(const SaveHandle& handle){
        //The world is in a state where serialisation or deserialisation is already taking place.
        if(!WorldSingleton::worldReady()) return;
        WorldSingleton::mWorldReady = false;
        mCurrentWorldState = WorldState::WORLD_STATE_SERALISE;
        mTargetSaveHandle = handle;

        WorldEventBecameUnReady event;
        EventDispatcher::transmitEvent(EventType::World, event);

        BaseSingleton::getSerialisationManager()->prepareSaveDirectory(handle);

        SerialisationManager::SaveInfoData data;
        data.playerPos = WorldSingleton::getPlayerPosition();
        data.mapName = WorldSingleton::getCurrentMap();
        BaseSingleton::getSerialisationManager()->writeDataToSaveFile(handle, data);

        serialisationJobCounter = 0;

        //Determine what meshes are in the world and gather a data list.
        mMeshSerialisationBuilder->prepareSerialisationMeshData();

        EntitySerialisationJob* entity = new EntitySerialisationJob(handle, &serialisationJobCounter, mEntityManager, mMeshSerialisationBuilder);

        JobDispatcher::dispatchJob(entity);
    }

    void World::_deserialise(const SaveHandle& handle){
        if(WorldSingleton::worldReady()) return;
        mCurrentWorldState = WorldState::WORLD_STATE_DESERALISE;

        SerialisationManager::SaveInfoData data;
        BaseSingleton::getSerialisationManager()->getDataFromSaveFile(handle, data);

        //TODO some of these things are causing the radiusLoader to load and unload things it doesn't need to on startup.
        mSlotManager->setCurrentMap(data.mapName);
        WorldSingleton::setPlayerPosition(data.playerPos);
        WorldSingleton::setPlayerLoadRadius(data.playerLoadRadius);

        serialisationJobCounter = 0;

        mEntityMeshStore = new SerialiserStringStore();
        mEntityScriptStore = new SerialiserStringStore();

        EntityDeSerialisationJob* entity = new EntityDeSerialisationJob(handle, &serialisationJobCounter, mEntityManager, mEntityMeshStore, mEntityScriptStore);
        JobDispatcher::dispatchJob(entity);
    }

    World::~World(){
        mSlotManager->shutdown();

        WorldEventDestroyed event;
        EventDispatcher::transmitEvent(EventType::World, event);
    }

    void World::update(){
        if(WorldSingleton::worldReady()){
            mSlotManager->update();
            mPhysicsManager->update();
            mEntityManager->update();
        }else{
            //The world is not ready, so we need to do checks as to whether it's become ready.
            if(serialisationJobCounter >= 1){
                //The serialisation has completed.
                if(mCurrentWorldState == WorldState::WORLD_STATE_DESERALISE){
                    _finishDeSerialisation();
                }
                else if(mCurrentWorldState == WorldState::WORLD_STATE_SERALISE){
                    _finishSerialisation();
                }
                mCurrentWorldState = WorldState::WORLD_STATE_READY;
                WorldSingleton::mWorldReady = true;

                WorldEventBecameReady event;
                EventDispatcher::transmitEvent(EventType::World, event);
            }
        }
    }

    void World::_finishSerialisation(){
        mMeshSerialisationBuilder->writeMeshFile(mTargetSaveHandle);
    }

    void World::_finishDeSerialisation(){
        for(auto i : mEntityMeshStore->mStoredStrings){
            AV_INFO("Creating ogre mesh {}", i.second);
            OgreMeshComponentLogic::add(i.first, i.second);
        }
        for(auto i : mEntityScriptStore->mStoredStrings){
            AV_INFO("Creating script with entity {}", i.second);
            ScriptComponentLogic::add(i.first, i.second);
        }

        delete mEntityMeshStore;
        delete mEntityScriptStore;
    }
}
