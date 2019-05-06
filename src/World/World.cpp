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

#include "Threading/JobDispatcher.h"
#include "Threading/Jobs/EntitySerialisationJob.h"

#include "Entity/EntityManager.h"

namespace AV {
    World::World(){

        mSlotManager = std::make_shared<SlotManager>();
        mChunkRadiusLoader = std::make_shared<ChunkRadiusLoader>(mSlotManager);

        mEntityManager = std::make_shared<EntityManager>();
        mEntityManager->initialise();

        WorldSingleton::setPlayerPosition(SlotPosition());
    }

    World::World(const SaveHandle& handle){

    }

    void World::serialise(const SaveHandle& handle){
        //The world is in a state where serialisation or deserialisation is already taking place.
        if(!WorldSingleton::worldReady()) return;
        WorldSingleton::mWorldReady = false;

        serialisationJobCounter = 0;

        EntitySerialisationJob* entity = new EntitySerialisationJob(handle, &serialisationJobCounter, mEntityManager);

        JobDispatcher::dispatchJob(entity);
    }

    void World::deserialise(const SaveHandle& handle){
        if(!WorldSingleton::worldReady()) return;
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
                WorldSingleton::mWorldReady = true;
            }
        }
    }
}
