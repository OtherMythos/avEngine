#pragma once

#include <memory>
#include <vector>
#include <deque>

#include "Recipe/RecipeData.h"

namespace AV{
    class ChunkFactory;
    class Chunk;
    class ChunkCoordinate;
    class SlotPosition;

    /**
    A class to manage the streamable open world.
    Deals with the management of chunks in slots, as well as chunk recipies.

    @remarks
    This class does not keep track of what needs to be loaded or unloaded based on the player position, and only does minimal sanity checks.
    It is a 'dumb' class which does exactly what it's told, working in tandem with the ChunkRadiusLoader to perform its operations.
    */
    class SlotManager{
    public:
        SlotManager();

        SlotManager(std::shared_ptr<ChunkFactory> factory);

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
        Deactivate an active chunk.

        @remarks
        If a chunk is pending activation on its loading job it will have its pending activation removed. However, the chunk will still be constructed.
        If you want the chunk entirely disposed of you should call DestroyChunk.

        @return True or false if the chunk was deactivated successfully.
        */
        bool deActivateChunk(const ChunkCoordinate &coord);

        /**
        Construct a chunk and place it in the loaded slot. This function won't activate a chunk. A separate call to activateChunk must be made.
        The intended purpose for this function is forward loading something.
        When a chunk is constructed it's as close to complete as it can be before activation.
        */
        bool constructChunk(const ChunkCoordinate &coord);

        /**
        Destroy a chunk that has already been constructed.
        */
        bool destroyChunk(const ChunkCoordinate &coord);

        /**
        Update the slot manager.
        This function should be called each tick, however if there is nothing to update it will return immediately.
        */
        void update();

        /**
         Set the current map of the world.

         @return
         True or False as to whether the map was switched sucessfully.
         */
        bool setCurrentMap(const Ogre::String& map);

        /**
        Set the origin of the world.
        The provided position will become the centre of the world.

        @param pos
        The new origin position.

        @remarks
        This will involve a shift of any active chunks in the world.
        */
        bool setOrigin(const SlotPosition &pos);

    private:
        std::shared_ptr<ChunkFactory> mChunkFactory;

        typedef std::pair<ChunkCoordinate, Chunk*> ChunkEntry;

        enum class QueuedRecipeType{
            RecipeTypeNone,
            RecipeTypeActivate,
            RecipeTypeConstruct
        };

        int mNextBlankRecipe = 0;
        static const int _MaxRecipies = 30;
        int _updateNeededCount = 0;

        typedef std::pair<ChunkCoordinate, QueuedRecipeType> QueueEntry;

        std::deque<QueueEntry> queuedEntries;

        RecipeData _recipeContainer[_MaxRecipies];

        //An array of recipies which want activation when they're done processing. 1 in this means the recipe at that index in _recipeContainer wants activation.
        bool _activationList[_MaxRecipies] = {};
        //An array of recipies which want construction when they're done processing.
        bool _constructionList[_MaxRecipies] = {};

        std::vector<ChunkEntry> mTotalChunks;

        bool _handleChunkRequest(const ChunkCoordinate &coord, bool activate);

        void _repositionChunks();

        /**
         Whether or not an update to the slot manager is necessary.

         @return
         True if an update should occur, false if not.
         */
        bool _updateNeeded() const;

        /**
        Find a chunk from the chunks list.

        @param coord
        The chunk to search for.

        @return
        A pointer to the chunk. 0 if a chunk with that coordinate couldn't be found.
        */
        Chunk* _findChunk(const ChunkCoordinate &coord) const;

        /**
        Internally construct a chunk.

        @remarks
        This function will always return a chunk, however it might not always be new.
        If a chunk coordinate is requested which is already constructed and in the loaded slot is requested then that will be returned.

        @param recipe The index of the recipe to construct from.
        @param positionChunk Whether or not to create the chunk in place.

        @return
        A pointer to the constructed chunk.
        */
        Chunk* _constructChunk(int recipe, bool positionChunk = false);

        /**
        Internally activate the chunk.
        This function expects a valid recipe. It makes no attempt to validate it.

        @param recipe
        The index of the recipe to activate from.
        */
        void _activateChunk(int recipe);

        /**
        Determine if that chunk has a recipe which is set to be activated.

        @return
        The index of the recipe in the recipies container.
        */
        int _chunkInActivationList(const ChunkCoordinate &coord);

        /**
        Check if a chunk coordinate request exists in the queue.

        @param coord
        The coordinate to check.

        @return
        True if the request was found, false if not.
        */
        std::deque<QueueEntry>::iterator _requestInQueue(const ChunkCoordinate &coord);

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
        int _findNextBlank(int start) const;

        /**
        Find a recipe entry and claim it, removing the previous recipe if required, and performing any necessary book keeping.
        This should be called when a new recipe needs to be created.

        @return
        The index of the claimed recipe.
        */
        int _claimRecipeEntry();
        /**
        Find a recipe entry. Don't change it in any way.
        If no recipie slot could be found, -1 is returned.

        @return
        The index where a new recipe can be inserted. If no slot could be found -1 is returned.
        */
        int _obtainRecipeEntry();
        /**
        Internal function to load a recipe. If a space in the recipe list is available, this function starts the jobs to load the recipe.
        If no space can be found the function will queue the request and return -1.

        @param loadType
        The type of load to perform. This is only considered if the request has to be queued.

        @return
        The index where the recipe was loaded into. -1 if the request had to be queued.
        */
        int _loadRecipe(const ChunkCoordinate &coord, QueuedRecipeType loadType);
        /**
        Age all the recipies by one.
        This should be called each time a new recipe is loaded.
        */
        void _incrementRecipeScore();
        /**
        Whether or not the recipe at the target index is still processing.

        @remarks
        Processing is when the recipe is still being constructed by thread jobs.
        When in this state the recipe should be considered untouchable.
        A recipe is considered to be processing when the slot is not available (has a recipe in it), and the recipe isn't ready(some jobs haven't finished).

        @return
        Whether or not the recipe at that index is processing.
        */
        bool _recipeProcessing(int loc);
        /**
         Reset and clear the values of a recipe entry.

         @param targetIndex
         The index of the recipe to clear.
         */
        void _clearRecipeEntry(int targetIndex);

        /**
        Determine a suitable replacement index in the recipies list.

        @remarks
        This function has the potential to not find a suitable place if all the recipies are pending.
        In this case -1 will be returned.

        @return
        The index to replace if one could be found. -1 if not.
        */
        int _determineReplacementIndex();
    };
}
