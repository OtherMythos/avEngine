#pragma once

namespace Ogre{
    class SceneManager;
    class SceneNode;
}

namespace AV{
    class RecipeData;
    class Chunk;

    class ChunkFactory{
    public:
        ChunkFactory();
        ~ChunkFactory();

        /**

        */
        Chunk* constructChunk(const RecipeData &recipe);

        /**
        Deconstruct a chunk, freeing up all the bits of memory allocated to it.

        @parm chunk
        The chunk to be deconstructed.

        @return
        Whether or not the chunk was deconstructed successfully.
        */
        bool deconstructChunk(Chunk* chunk);

    private:
        Ogre::SceneManager *mSceneManager;

        Ogre::SceneNode *mStaticShapeNode;
    };
}
