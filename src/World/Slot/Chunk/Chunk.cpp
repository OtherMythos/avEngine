#include "Chunk.h"

#include "OgreSceneNode.h"
#include "OgreSceneManager.h"
#include "World/Slot/SlotPosition.h"

#include "World/Physics/PhysicsManager.h"
#include "World/Physics/Worlds/DynamicsWorld.h"
#include "World/Physics/Worlds/CollisionWorld.h"
#include "World/Physics/PhysicsBodyDestructor.h"
#include "System/SystemSetup/SystemSettings.h"

#include "Terrain/Terrain.h"

namespace AV{
    Chunk::Chunk(const ChunkCoordinate &coord, std::shared_ptr<PhysicsManager> physicsManager, Ogre::SceneManager *sceneManager, Ogre::SceneNode *staticMeshes, PhysicsTypes::PhysicsChunkEntry physicsChunk, PhysicsTypes::CollisionChunkEntry collisionChunk, Terrain* terrain)
    : mChunkCoordinate(coord),
    mSceneManager(sceneManager),
    mStaticMeshes(staticMeshes),
    mPhysicsManager(physicsManager),
    mPhysicsChunk(physicsChunk),
    mCollisionChunk(collisionChunk),
    mTerrain(terrain) {

    }

    Chunk::~Chunk(){
        //TODO this should be moved to the ChunkFactory deconstructChunk.
        if(mPhysicsChunk == PhysicsTypes::EMPTY_CHUNK_ENTRY) return;

        if(mActive){
            //The false means not to request the object removal from the threaded world. This will be done by the destructor.
            if(!SystemSettings::getDynamicPhysicsDisabled()){
                mPhysicsManager->getDynamicsWorld()->removePhysicsChunk(currentPhysicsChunk, false);
            }
        }
        PhysicsBodyDestructor::destroyPhysicsWorldChunk(mPhysicsChunk);
    }

    void Chunk::activate(){
        if(mActive) return;

        mStaticMeshes->setVisible(true);

        if(mCollisionChunk != PhysicsTypes::EMPTY_COLLISION_CHUNK_ENTRY){
            currentCollisionObjectChunk = CollisionWorld::addCollisionObjectChunk(mCollisionChunk);
        }
        if(mPhysicsChunk != PhysicsTypes::EMPTY_CHUNK_ENTRY){
            assert(!SystemSettings::getDynamicPhysicsDisabled());
            currentPhysicsChunk = mPhysicsManager->getDynamicsWorld()->addPhysicsChunk(mPhysicsChunk);
        }
        if(mTerrain){
            mPhysicsManager->getDynamicsWorld()->addTerrainBody(mTerrain->getTerrainBody(), mChunkCoordinate.chunkX(), mChunkCoordinate.chunkY());
        }

        mActive = true;
    }

    void Chunk::deActivate(){
        if(!mActive) return;

        if(mPhysicsChunk != PhysicsTypes::EMPTY_CHUNK_ENTRY){
            assert(!SystemSettings::getDynamicPhysicsDisabled());
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

        if(mTerrain){
            Ogre::SceneNode* terrainNode = mTerrain->getTerrainSceneNode();
            mStaticMeshes->setPosition(targetPos);
            mSceneManager->notifyStaticDirty(terrainNode);

            mTerrain->setTerrainPosition(targetPos);
        }

        //TODO this needs an update for the new system.
        /*auto it = mStaticMeshes->getChildIterator();
        while (it.hasMoreElements()) {
            Ogre::SceneNode *node = (Ogre::SceneNode*)it.getNext();

            Ogre::MovableObject* object = node->getAttachedObject(0);
            mSceneManager->notifyStaticAabbDirty(object);
        }*/
    }
};
