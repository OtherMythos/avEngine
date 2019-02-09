#pragma once

namespace Ogre{
    class SceneManager;
    class SceneNode;
}

namespace AV{
    struct RecipeData;
    class Chunk;

    /**
    A factory class responsible for constructing chunks from a recipe.
    */
    class ChunkFactory{
    public:
        ChunkFactory();
        virtual ~ChunkFactory();

        void initialise();

        /**
        Start an ogre recipe job for that recipe reference.

        @param data
        The recipe data to process.
        */
        virtual void startRecipeJob(RecipeData* data);

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

        void reposition();

    private:
        Ogre::SceneManager *mSceneManager;

        //The parent node of all chunk shapes to be created.
        Ogre::SceneNode *mStaticShapeNode;
    };
}
