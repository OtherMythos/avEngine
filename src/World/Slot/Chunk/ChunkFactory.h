#pragma once

namespace Ogre{
    class SceneManager;
    class SceneNode;
}

namespace AV{
    class RecipeData;
    class Chunk;

    /**
    A factory class responsible for constructing chunks from a recipe.
    */
    class ChunkFactory{
    public:
        ChunkFactory();
        ~ChunkFactory();

        void initialise();

        /**
        Construct a chunk from a recipe.

        @param recipe
        The recipe to construct.

        @param position
        Whether or not the chunk should be positioned at its destination position, this being whatever the chunk position relative to the origin is.
        If false the chunk will be set exactly to the origin, which is the same as the loaded slot.
        */
        virtual Chunk* constructChunk(const RecipeData &recipe, bool position = false);
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

        //The parent node of all chunk shapes to be created.
        Ogre::SceneNode *mStaticShapeNode;
    };
}
