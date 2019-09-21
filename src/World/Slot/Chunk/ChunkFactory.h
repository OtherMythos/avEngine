#pragma once

#include "World/Slot/Recipe/RecipeData.h"
#include "Threading/JobId.h"

namespace Ogre{
    class SceneManager;
    class SceneNode;
}

namespace AV{
    struct RecipeData;
    class Chunk;
    class PhysicsManager;
    class PhysicsBodyConstructor;
    class TerrainManager;

    /**
    A factory class responsible for constructing chunks from a recipe.
    */
    class ChunkFactory{
    public:
        ChunkFactory(std::shared_ptr<PhysicsManager> physicsManager, std::shared_ptr<TerrainManager> terrainManager);
        virtual ~ChunkFactory();

        void initialise();

        /**
        Start an ogre recipe job for that recipe reference.

        @param data
        The recipe data to process.
        */
        virtual void startRecipeJob(RecipeData* data, int targetIndex);

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

        void getTerrainTestData(int& inUseTerrains, int& availableTerrains);

        void shutdown();

    private:
        Ogre::SceneManager *mSceneManager;

        //The parent node of all chunk shapes to be created.
        Ogre::SceneNode *mStaticShapeNode = 0;

        JobId mRunningMeshJobs[RecipeData::MaxRecipies];
        JobId mRunningBodyJobs[RecipeData::MaxRecipies];

        std::shared_ptr<PhysicsManager> mPhysicsManager;
        std::shared_ptr<TerrainManager> mTerrainManager;
    };
}
