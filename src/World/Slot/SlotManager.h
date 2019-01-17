#pragma once

#include "Recipe/RecipeData.h"

namespace AV{
    class ChunkCoordinate;

    class SlotManager{
    public:
        SlotManager();

        void initialise();

        bool loadChunk(const ChunkCoordinate &coord);
        bool unloadChunk(const ChunkCoordinate &coord);

        bool activateChunk(const ChunkCoordinate &coord);

        void update();

    private:
        int _recipeCount = 0;
        int _nextBlankRecipe = 0;
        static const int _MaxRecipies = 10;
        int _updateNeededCount = 0;

        RecipeData _recipeContainer[_MaxRecipies];

        //An array of recipies which are still processing. A 1 in this array means the recipe of that index in the _recipeContainer is still processing.
        bool _processingList[_MaxRecipies] = {};
        //An array of recipies which want activation when they're done processing. 1 in this means the recipe at that index in _recipeContainer wants activation.
        bool _activationList[_MaxRecipies] = {};

        void _activateChunk(int recipe);

        /**
        Check for a recipe and return it if found.

        @return
        -1 if the recipe was not found in the list.
        A number between 0 and (_MaxRecipies -1) if one was found, representing the index of the found recipe.
        */
        int _recipeLoaded(const ChunkCoordinate &coord);

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

        /**
        Find a recipe entry and claim it, removing the previous recipe if required, and performing any necessary book keeping.
        This should be called when a new recipe needs to be created.
        */
        int _claimRecipeEntry();
        int _obtainRecipeEntry();
        int _findHighestScoringRecipe();
        int _loadRecipe(const ChunkCoordinate &coord);
        void _incrementRecipeScore();
    };
}
