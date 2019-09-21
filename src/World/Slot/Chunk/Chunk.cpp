#include "Chunk.h"

#include "OgreSceneNode.h"
#include "OgreSceneManager.h"
#include "World/Slot/SlotPosition.h"

#include "World/Physics/PhysicsManager.h"
#include "World/Physics/Worlds/DynamicsWorld.h"
#include "World/Physics/PhysicsBodyDestructor.h"

#include "Terrain/Terrain.h"

namespace AV{
    Chunk::Chunk(const ChunkCoordinate &coord, std::shared_ptr<PhysicsManager> physicsManager, Ogre::SceneManager *sceneManager, Ogre::SceneNode *staticMeshes, PhysicsTypes::PhysicsChunkEntry physicsChunk, Terrain* terrain)
    : mChunkCoordinate(coord),
    mSceneManager(sceneManager),
    mStaticMeshes(staticMeshes),
    mPhysicsManager(physicsManager),
    mPhysicsChunk(physicsChunk),
    mTerrain(terrain) {

    }

    Chunk::~Chunk(){
        //TODO this should be moved to the ChunkFactory deconstructChunk.
        if(mPhysicsChunk == PhysicsTypes::EMPTY_CHUNK_ENTRY) return;

        if(mActive){
            //The false means not to request the object removal from the threaded world. This will be done by the destructor.
            mPhysicsManager->getDynamicsWorld()->removePhysicsChunk(currentPhysicsChunk, false);
        }
        PhysicsBodyDestructor::destroyPhysicsWorldChunk(mPhysicsChunk);
    }

    void Chunk::activate(){
        if(mActive) return;

        mStaticMeshes->setVisible(true);

        if(mPhysicsChunk != PhysicsTypes::EMPTY_CHUNK_ENTRY){
            currentPhysicsChunk = mPhysicsManager->getDynamicsWorld()->addPhysicsChunk(mPhysicsChunk);
        }

        mActive = true;
    }

    void Chunk::deActivate(){
        if(!mActive) return;

        if(mPhysicsChunk != PhysicsTypes::EMPTY_CHUNK_ENTRY){
            mPhysicsManager->getDynamicsWorld()->removePhysicsChunk(currentPhysicsChunk);
        }

        mStaticMeshes->setVisible(false);
        mActive = false;
    }

    void Chunk::update(){
        if(mTerrain)
            mTerrain->update();
    }

    void Chunk::reposition(){
        SlotPosition pos(mChunkCoordinate.chunkX(), mChunkCoordinate.chunkY());

        Ogre::Vector3 targetPos = pos.toOgre();
        mStaticMeshes->setPosition(targetPos);
        mSceneManager->notifyStaticDirty(mStaticMeshes);

        auto it = mStaticMeshes->getChildIterator();
        while (it.hasMoreElements()) {
            Ogre::SceneNode *node = (Ogre::SceneNode*)it.getNext();

            Ogre::MovableObject* object = node->getAttachedObject(0);
            mSceneManager->notifyStaticAabbDirty(object);
        }
    }
};
