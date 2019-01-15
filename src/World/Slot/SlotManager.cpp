#include "SlotManager.h"

#include "Logger/Log.h"

#include "Threading/JobDispatcher.h"
#include "Threading/Jobs/RecipeOgreMeshJob.h"

namespace AV{
    SlotManager::SlotManager(){

    }

    void SlotManager::initialise(){

    }

    bool SlotManager::loadChunk(const ChunkCoordinate &coord){
        AV_INFO("Loading chunk");
        //If the recipe is already loaded, don't do anything.
        if(_recipeLoaded(coord)) return false;

        //Get a position in the array.
        for(int i = 0; i < 15; i++){
            int targetIndex = _claimRecipeEntry();

            _recipeContainer[targetIndex].coord = coord;

            //_recipeContainer[targetIndex].ogreMeshData = new std::vector<int>();
            JobDispatcher::dispatchJob(new RecipeOgreMeshJob(&_recipeContainer[targetIndex]));
        }

        // int recipeSlot = _obtainRecipeEntry();
        // AV_INFO(recipeSlot);
        // AV_INFO(_nextBlankRecipe);
        //
        // for(int i = 0; i < 10; i++){
        //     _recipeContainer[i].slotAvailable = false;
        // }
        // recipeSlot = _obtainRecipeEntry();
        // AV_INFO(recipeSlot);
        // AV_INFO(_nextBlankRecipe);



        // int next = _findNextBlank(0);
        // AV_INFO("Next entry {}", next);
        //
        // _recipeContainer[0].slotAvailable = false;
        // _recipeContainer[1].slotAvailable = false;
        //
        // next = _findNextBlank(0);
        // AV_INFO("Next entry {}", next);
        //
        // for(int i = 0; i < 10 - 1; i++){
        //     _recipeContainer[i].slotAvailable = false;
        // }
        //
        // _recipeContainer[5].slotAvailable = true;
        //
        // next = _findNextBlank(0);
        // AV_INFO("Next entry {}", next);
        //
        // next = _findNextBlank(5);
        // AV_INFO("Next entry {}", next);
        //
        // for(int i = 0; i < 10; i++){
        //     _recipeContainer[i].slotAvailable = false;
        // }
        // _recipeContainer[3].slotAvailable = true;
        //
        // next = _findNextBlank(5);
        // AV_INFO("Next entry {}", next);

        _recipeCount++;

        return true;
    }

    bool SlotManager::unloadChunk(const ChunkCoordinate &coord){
        return true;
    }

    bool SlotManager::_recipeLoaded(const ChunkCoordinate &coord){
        for(int i = 0; i < _recipeCount; i++){
            if(coord == _recipeContainer[_recipeCount].coord) return true;
        }
        return false;
    }

    void SlotManager::_incrementRecipeScore(){
        for(int i = 0; i < _MaxRecipies; i++){
            //If the slot is available, i.e the slot contains no recipe then don't bother increasing the score.
            if(_recipeContainer[i].slotAvailable) continue;

            _recipeContainer[i].recipeScore = _recipeContainer[i].recipeScore + 1;
        }
    }

    int SlotManager::_claimRecipeEntry(){
        int targetIndex = _obtainRecipeEntry();
        _incrementRecipeScore();

        //reset the values in that recipe.
        _recipeContainer[targetIndex].recipeScore = 0;
        _recipeContainer[targetIndex].slotAvailable = false;
        _recipeContainer[targetIndex].coord = ChunkCoordinate();

        //TODO write a thing to tell if a vector already exists and delete it.
        _recipeContainer[targetIndex].ogreMeshData = 0;
        _recipeContainer[targetIndex].jobDoneCounter = 0;

        return targetIndex;
    }

    int SlotManager::_obtainRecipeEntry(){
        int retPos = _nextBlankRecipe;
        _nextBlankRecipe = _findNextBlank(retPos);

        if(retPos == -1){
            //No recipe could be found. This means we need to make room.
            int removalIndex = _findHighestScoringRecipe();

            retPos = removalIndex;
        }

        return retPos;
    }

    int SlotManager::_findHighestScoringRecipe(){
        int highestIndex = 0;
        int highestScore = 0;

        for(int i = 0; i < _MaxRecipies; i++){
            if(_recipeContainer[i].recipeScore > highestScore){
                highestScore = _recipeContainer[i].recipeScore;
                highestIndex = i;
            }
        }

        return highestIndex;
    }

    int SlotManager::_findNextBlank(int start){
        //Don't check -1.
        if(start == -1) return start;

        //We increment start so we only check the values after the given entry.
        start++;
        //Iterate the rest of the array.
        for(int i = 0; i < _MaxRecipies - start; i++){
            int index = i+start;
            if(_recipeContainer[index].slotAvailable) return index;
        }
        //If no next place can be found return this to specify that.
        return -1;
    }
};
