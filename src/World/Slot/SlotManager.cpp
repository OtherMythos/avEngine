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
        if(_recipeLoaded(coord) != -1) return false;

        _loadRecipe(coord);

        return true;
    }

    bool SlotManager::unloadChunk(const ChunkCoordinate &coord){
        return true;
    }

    bool SlotManager::activateChunk(const ChunkCoordinate &coord){
        //TODO some check as to whether that chunk is activated anyway would be good.
        int targetRecipe = _recipeLoaded(coord);
        if(targetRecipe != -1){
            //The recipe is loaded.
            //Check if the recipe is ready.
            if(_recipeContainer[targetRecipe].recipeReady)
                _activateChunk(targetRecipe);
            else{
                //If the chunk isn't ready, add it to the activation list to be processed later.
                ///It should already be in the processing list.
                _activationList[targetRecipe] = true;
                _updateNeededCount++;
            }
        }else{
            //The recipe is not loaded.
            //Load it.
            int recipeIndex = _loadRecipe(coord);

            //The load job has now started.
            //Add the recipe to the activation list.
            _activationList[recipeIndex] = true;
        }
    }

    void SlotManager::update(){
        //There are no recipies waiting for update, so don't bother updating.
        if(_updateNeededCount <= 0) return;

        int traversalAmmount = _updateNeededCount;
        for(int i = 0; i < _MaxRecipies; i++){
            if(_processingList[i]){
                //Check the progress.
                if(_recipeContainer[i].jobDoneCounter >= RecipeData::targetJobs){
                    //The recipe is finished processing.
                    _recipeContainer[i].recipeReady = true;
                    _processingList[i] = false;
                    _updateNeededCount--;

                    //Check if that recipe needs to be activated.
                    if(_activationList[i]){
                        _activateChunk(i);
                        _activationList[i] = false;
                    }
                }

                traversalAmmount--;
                //If all the recipies which need updating have been processed then we can break.
                if(traversalAmmount <= 0) break;
            }
        }
    }

    void SlotManager::_activateChunk(int recipe){
        AV_INFO("Activating chunk");
    }

    int SlotManager::_loadRecipe(const ChunkCoordinate &coord){
        //Get a position in the array.
        int targetIndex = _claimRecipeEntry();

        _recipeContainer[targetIndex].coord = coord;

        //_recipeContainer[targetIndex].ogreMeshData = new std::vector<int>();
        JobDispatcher::dispatchJob(new RecipeOgreMeshJob(&_recipeContainer[targetIndex]));

        _processingList[targetIndex] = true;
        _updateNeededCount++;

        _recipeCount++;

        return targetIndex;
    }

    int SlotManager::_recipeLoaded(const ChunkCoordinate &coord){
        for(int i = 0; i < _recipeCount; i++){
            if(coord == _recipeContainer[_recipeCount].coord) return i;
        }
        return -1;
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
        _recipeContainer[targetIndex].recipeReady = false;

        //TODO write a thing to tell if a vector already exists and delete it.
        _recipeContainer[targetIndex].ogreMeshData = 0;
        _recipeContainer[targetIndex].jobDoneCounter = 0;


        _processingList[targetIndex] = false;
        _activationList[targetIndex] = false;

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
