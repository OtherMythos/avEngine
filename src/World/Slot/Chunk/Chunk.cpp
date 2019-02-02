#include "Chunk.h"

#include "OgreSceneNode.h"
#include "World/Slot/SlotPosition.h"

namespace AV{
    Chunk::Chunk(const ChunkCoordinate &coord, Ogre::SceneNode *staticMeshes)
    : mChunkCoordinate(coord),
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

        mStaticMeshes->setPosition(pos.toOgre());
        //mStaticMeshes->setPosition(Ogre::Vector3::ZERO);
    }
};
