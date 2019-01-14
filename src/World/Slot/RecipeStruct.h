#pragma once

#include "ChunkCoordinate.h"

namespace AV{
    struct RecipeData{
        ChunkCoordinate coord;

        //Whether this slot can be used to create a recipe.
        bool slotAvailable = true;

        int recipeScore = 0;
    };
}
