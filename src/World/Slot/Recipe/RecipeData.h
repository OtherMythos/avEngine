#pragma once

#include "World/Slot/ChunkCoordinate.h"
#include "OgreMeshRecipeData.h"
#include "PhysicsBodyRecipeData.h"

#include <atomic>
#include <vector>
#include <OgreVector3.h>

namespace AV{
    //TODO maybe put these into a different file.
    enum class SceneType{
        child,
        empty,
        term,
        mesh
    };

    struct RecipeSceneEntry{
        SceneType type;
        unsigned int id;
        Ogre::Vector3 pos;
        Ogre::Vector3 scale;
    };

    struct RecipeData{
        RecipeData() : jobDoneCounter(0) { };

        ChunkCoordinate coord = ChunkCoordinate();

        //Whether this slot can be used to create a recipe.
        bool slotAvailable = true;

        //Whether or not this recipe has had all its loading jobs completed and is ready to be constructed.
        bool recipeReady = false;

        //Score used to keep track of the age of the recipe. This is used by the SlotManager to determine which slots are the oldest.
        int recipeScore = 0;

        //Counter to determine how done a recipe is. This will be incremented by the jobs. When this is equal to the targetJobs the SlotManager will mark this recipe as ready.
        std::atomic<int> jobDoneCounter;

        //The parsed scene tree entries.
        std::vector<RecipeSceneEntry>* sceneEntries = 0;

        //Data of the static ogre meshes which should be inserted into the scene.
        std::vector<OgreMeshRecipeData>* ogreMeshData = 0;

        //Vector pointer to the physics body data constructed by the thread job. This might be 0 if the job failed.
        std::vector<PhysicsBodyRecipeData> *physicsBodyData = 0;
        //Recipe data for shapes that should be created.
        std::vector<PhysicsShapeRecipeData> *physicsShapeData = 0;

        //The number of jobs a recipe contains.
        //Currently includes meshes, physics shapes.
        static const int targetJobs = 2;
        static const int MaxRecipies = 10;
    };
}
