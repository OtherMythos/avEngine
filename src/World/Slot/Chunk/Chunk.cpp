#include "Chunk.h"

#include "OgreSceneNode.h"
#include "OgreSceneManager.h"
#include "World/Slot/SlotPosition.h"

#include "World/Physics/PhysicsManager.h"
#include "World/Physics/Worlds/DynamicsWorld.h"

namespace AV{
    Chunk::Chunk(const ChunkCoordinate &coord, std::shared_ptr<PhysicsManager> physicsManager, Ogre::SceneManager *sceneManager, Ogre::SceneNode *staticMeshes, PhysicsTypes::PhysicsChunkEntry physicsChunk)
    : mChunkCoordinate(coord),
    mSceneManager(sceneManager),
    mStaticMeshes(staticMeshes),
    mPhysicsManager(physicsManager),
    mPhysicsChunk(physicsChunk) {

    }

    Chunk::~Chunk(){
        //TODO Here I should have some procedure to delete the chunk btRigidBody pointers.
        //For now just remove them from the world.
        if(mActive && mPhysicsChunk != PhysicsTypes::EMPTY_CHUNK_ENTRY){
            mPhysicsManager->getDynamicsWorld()->removePhysicsChunk(currentPhysicsChunk);
        }
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
