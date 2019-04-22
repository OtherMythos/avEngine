#pragma once

#include "World/Slot/ChunkCoordinate.h"
#include "OgreMeshRecipeData.h"

#include <atomic>
#include <vector>

namespace AV{
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

        //Vector pointer to the Ogre mesh recipe data constructed by the thread job. This might be 0 if the job failed.
        std::vector<OgreMeshRecipeData> *ogreMeshData = 0;

        static const int targetJobs = 1;
        static const int MaxRecipies = 10;
    };
}
