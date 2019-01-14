#pragma once

#include "RecipeStruct.h"

namespace AV{
    class ChunkCoordinate;

    class SlotManager{
    public:
        SlotManager();

        void initialise();

        bool loadChunk(const ChunkCoordinate &coord);
        bool unloadChunk(const ChunkCoordinate &coord);

    private:
        int _recipeCount = 0;
        int _nextBlankRecipe = 0;
        static const int _MaxRecipies = 10;

        RecipeData _recipeContainer[_MaxRecipies];

        bool _recipeLoaded(const ChunkCoordinate &coord);
        /**
        Find the next blank space in the recipies list.

        @remarks
        The search starts from a position in the index and carries on to the right.
        It will only search to the right.
        If there is a space on the left side of the start position, the search won't find it and -1 will be returned.

        @param start
        The position in which to start the search

        @return
        The index of the next available space. -1 if there are none.
        */
        int _findNextBlank(int start);
        int _obtainRecipeEntry();
        int _findHighestScoringRecipe();
        void _incrementRecipeScore();
    };
}
