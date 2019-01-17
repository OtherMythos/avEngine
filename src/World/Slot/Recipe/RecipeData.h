#pragma once

#include "World/Slot/ChunkCoordinate.h"
#include "OgreMeshRecipeData.h"

#include <atomic>
#include <vector>

namespace AV{
    struct RecipeData{
        RecipeData() : jobDoneCounter(0) { };

        ChunkCoordinate coord;

        //Whether this slot can be used to create a recipe.
        bool slotAvailable = true;

        //Whether or not this recipe has had all its loading jobs completed and is ready to be constructed.
        bool recipeReady = false;

        int recipeScore = 0;

        std::atomic<int> jobDoneCounter;

        std::vector<OgreMeshRecipeData> *ogreMeshData = 0;

        static const int targetJobs = 1;
    };
}
