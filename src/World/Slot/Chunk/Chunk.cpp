#include "Chunk.h"

#include "OgreSceneNode.h"
#include "OgreSceneManager.h"
#include "World/Slot/SlotPosition.h"

namespace AV{
    Chunk::Chunk(const ChunkCoordinate &coord, Ogre::SceneManager *sceneManager, Ogre::SceneNode *staticMeshes)
    : mChunkCoordinate(coord),
    mSceneManager(sceneManager),
    mStaticMeshes(staticMeshes) {

    }

    Chunk::~Chunk(){

    }

    void Chunk::activate(){
        if(mActive) return;

        mStaticMeshes->setVisible(true);
        mActive = true;
    }

    void Chunk::deActivate(){
        if(!mActive) return;

        mStaticMeshes->setVisible(false);
        mActive = false;
    }

    void Chunk::reposition(){
        SlotPosition pos(mChunkCoordinate.chunkX(), mChunkCoordinate.chunkY());

        Ogre::Vector3 targetPos = pos.toOgre();
        mStaticMeshes->setPosition(targetPos);
        mSceneManager->notifyStaticDirty(mStaticMeshes);
        //mSceneManager->notifyStaticDirty(mSceneManager->getRootSceneNode());
    }
};
