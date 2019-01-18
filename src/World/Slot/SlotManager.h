#pragma once

#include <memory>

#include "Recipe/RecipeData.h"

namespace AV{
    class ChunkFactory;

    class ChunkCoordinate;

    /**
    A class to manage the streamable open world.
    Deals with the management of chunks in slots, as well as chunk recipies.
    */
    class SlotManager{
    public:
        SlotManager();

        void initialise();

        /**
        Load a chunk recipe. This can be used to start the threaded job of loading the chunk data.
        This function will not create a chunk in the world.

        @remarks
        This function will load the chunk recipe into memory.
        This does not involve creation, and is intended to be used to forwarn the engine that a chunk will eventually be requested for creation.
        If more chunks are requested for loading than the designated ammount then some older recipies will be removed.
        Calling ActivateChunk will load the chunk recipe if necessary.

        @return
        True or false as to whether the chunk was loaded successfully or not.
        */
        bool loadChunk(const ChunkCoordinate &coord);
        /**
        Free up the space taken by a chunk recipe in memory.

        @remarks
        Chunk recipies are not normally destroyed until the space is needed.
        If you can't be sure that this recipe won't be needed again, there is no harm in not unloading it, as the engine will unload it when it needs to.
        It is more efficient to have chunks loaded into the designated memory allocation rather than none, just incase they are eventually needed.

        @return
        True or false as to whether the chunk was unloaded successfully or not.
        **/
        bool unloadChunk(const ChunkCoordinate &coord);

        /**
        Activate a chunk and insert it into the world.
        This function will search for a chunk recipe. If it can't find one it will load it.
        The time in which this function goes from call to active chunk depends on the time it takes the recipe loading jobs to complete.

        */
        bool activateChunk(const ChunkCoordinate &coord);

        /**
        Update the slot manager.
        This function should be called each tick, however if there is nothing to update it will return immediately.
        */
        void update();

    private:
        std::shared_ptr<ChunkFactory> mChunkFactory;

        int _recipeCount = 0;
        int _nextBlankRecipe = 0;
        static const int _MaxRecipies = 10;
        int _updateNeededCount = 0;

        RecipeData _recipeContainer[_MaxRecipies];

        //An array of recipies which are still processing. A 1 in this array means the recipe of that index in the _recipeContainer is still processing.
        bool _processingList[_MaxRecipies] = {};
        //An array of recipies which want activation when they're done processing. 1 in this means the recipe at that index in _recipeContainer wants activation.
        bool _activationList[_MaxRecipies] = {};

        /**
        Internally activate the chunk.
        This function expects a valid recipe. It makes no attempt to validate it.

        @param recipe
        The index of the recipe to activate from.
        */
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
        If there is a space on the left side of the start position, the search won't find it.

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
        /**
        Find a recipe entry. Don't change it in any way.

        @return
        The index where a new recipe can be inserted.
        */
        int _obtainRecipeEntry();
        int _findHighestScoringRecipe();
        /**
        Internal function to load a recipe. This function starts the jobs to load a recipe in.

        @return
        The index where the recipe was loaded into.
        */
        int _loadRecipe(const ChunkCoordinate &coord);
        /**
        Age all the recipies by one.
        This should be called each time a new recipe is loaded.
        */
        void _incrementRecipeScore();
    };
}
