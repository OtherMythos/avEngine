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

#include "Entity/EntityManager.h"

namespace AV {
    World::World(){

        mSlotManager = std::make_shared<SlotManager>();
        mChunkRadiusLoader = std::make_shared<ChunkRadiusLoader>(mSlotManager);

        mEntityManager = std::make_shared<EntityManager>();
        mEntityManager->initialise();

        WorldSingleton::setPlayerPosition(SlotPosition());
    }

    World::~World(){

    }

    void World::update(Ogre::Camera* camera){
        mSlotManager->update();
    }
}
