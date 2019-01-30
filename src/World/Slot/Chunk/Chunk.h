#pragma once

#include "World/Slot/ChunkCoordinate.h"

namespace Ogre{
    class SceneNode;
}

namespace AV{
    /**
    A constructed chunk in the world, derived from the construction instructions from a recipe.
    Chunks contain things such as static meshes, physics shapes, colliders and so on.
    Chunks provide an interface to activate and deactivate their contents.

    @remarks
    Chunks are constructed using the ChunkFactory, being given pointers to their constructed data to manage.
    */
    class Chunk{
    public:
        Chunk(const ChunkCoordinate &coord, Ogre::SceneNode *staticMeshes);
        ~Chunk();

        /**
        Sets the chunk to be active.
        This will set meshes to visible and enable physics shapes.
        */
        void activate();

        /**
        Deactivate the chunk.
        This will set the meshes to be invisible.
        */
        void deActivate();

        /**
        Reposition this chunk.

        @remarks
        This position will be determined based off of the current origin.
        This function should not be called unless this chunk is within the safe bounds of the world origin, otherwise floating point precision issues might occur.
        */
        void reposition();

        Ogre::SceneNode* getStaticMeshNode() { return mStaticMeshes; };

    private:
        ChunkCoordinate mChunkCoordinate;

        Ogre::SceneNode *mStaticMeshes;

        bool mActive = false;
    };
}
