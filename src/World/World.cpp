#include "World.h"

#include "Slot/SlotPosition.h"
#include "Slot/SlotManager.h"
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

#include "Entity/EntityManager.h"

namespace AV {
    World::World(){
        _initialise();
    }

    World::World(const SaveHandle& handle){
        _initialise();
        _deserialise(handle);
    }
    
    void World::_initialise(){
        mSlotManager = std::make_shared<SlotManager>();
        mChunkRadiusLoader = std::make_shared<ChunkRadiusLoader>(mSlotManager);

        mEntityManager = std::make_shared<EntityManager>();
        mEntityManager->initialise();

        WorldSingleton::setPlayerPosition(SlotPosition());
    }

    void World::serialise(const SaveHandle& handle){
        //The world is in a state where serialisation or deserialisation is already taking place.
        if(!WorldSingleton::worldReady()) return;
        WorldSingleton::mWorldReady = false;

        serialisationJobCounter = 0;

        EntitySerialisationJob* entity = new EntitySerialisationJob(handle, &serialisationJobCounter, mEntityManager);

        JobDispatcher::dispatchJob(entity);
    }

    void World::_deserialise(const SaveHandle& handle){
        if(WorldSingleton::worldReady()) return;
        
        serialisationJobCounter = 0;
        
        mEntityMeshStore = new SerialiserStringStore();
        mEntityScriptStore = new SerialiserStringStore();
        
        EntityDeSerialisationJob* entity = new EntityDeSerialisationJob(handle, &serialisationJobCounter, mEntityManager, mEntityMeshStore, mEntityScriptStore);
        JobDispatcher::dispatchJob(entity);
    }

    World::~World(){
        mSlotManager->shutdown();
    }

    void World::update(){
        if(WorldSingleton::worldReady()){
            mSlotManager->update();
        }else{
            //The world is not ready, so we need to do checks as to whether it's become ready.
            if(serialisationJobCounter >= 1){
                //The serialisation has completed.
                _finishDeSerialisation();
            }
        }
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
        WorldSingleton::mWorldReady = true;
    }
}
