#include "Chunk.h"

#include "OgreSceneNode.h"

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

    void Chunk::reposition(Ogre::Vector3 position){
        mStaticMeshes->setPosition(position);
    }
};
