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
    }

    void Chunk::deActivate(){
        if(!mActive) return;

        mStaticMeshes->setVisible(false);
    }

    void Chunk::reposition(){
        SlotPosition pos(mChunkCoordinate.chunkX(), mChunkCoordinate.chunkY());

        mSceneManager->notifyStaticDirty(mStaticMeshes);
        mStaticMeshes->setPosition(pos.toOgre());
    }
};
