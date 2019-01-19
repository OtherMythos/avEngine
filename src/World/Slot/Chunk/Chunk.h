#pragma once

#include "World/Slot/ChunkCoordinate.h"

namespace Ogre{
    class SceneNode;
}

namespace AV{
    class Chunk{
    public:
        Chunk(const ChunkCoordinate &coord, Ogre::SceneNode *staticMeshes);
        ~Chunk();

        /**
        Sets the chunk to be active.
        This will set meshes to visible and enable physics shapes.
        */
        void activate();
        void deActivate();

        /**
        Reposition this chunk to the provided position.

        @remarks
        This function doesn't take a slot position as the target position is supposed to be relative to the origin.
        */
        void reposition(Ogre::Vector3 position);

        Ogre::SceneNode* getStaticMeshNode() { return mStaticMeshes; };

    private:
        ChunkCoordinate mChunkCoordinate;

        Ogre::SceneNode *mStaticMeshes;

        bool mActive = false;
    };
}
