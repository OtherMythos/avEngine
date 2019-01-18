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

        Chunk* constructChunk(const RecipeData &recipe);

    private:
        Ogre::SceneManager *mSceneManager;

        Ogre::SceneNode *mStaticShapeNode;
    };
}
