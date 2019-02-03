#include "SlotManager.h"

#include "Logger/Log.h"

#include "Threading/JobDispatcher.h"
#include "Threading/Jobs/RecipeOgreMeshJob.h"
#include "World/WorldSingleton.h"

#include "Event/EventDispatcher.h"
#include "Event/Events/WorldEvent.h"

#include "Chunk/ChunkFactory.h"
#include "Chunk/Chunk.h"

namespace AV{
    SlotManager::SlotManager(){
        mChunkFactory = std::make_shared<ChunkFactory>();
        mChunkFactory->initialise();

        initialise();
    }

    SlotManager::SlotManager(std::shared_ptr<ChunkFactory> factory)
        : mChunkFactory(factory){
        initialise();
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
        //First check whether the chunk is already constructed.
        Chunk *chunk = _findChunk(coord);
        if(chunk){
            //The chunk is loaded so activate that.
            chunk->activate();
            return true;
        }
        //If the chunk is not already constructed then see what to do there.
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
            _updateNeededCount++;
        }
        return true;
    }

    bool SlotManager::destroyChunk(const ChunkCoordinate &coord){
        auto it = mTotalChunks.begin();
        while(it != mTotalChunks.end()){
            if(coord == (*it).first){
                mChunkFactory->deconstructChunk((*it).second);
                delete (*it).second;
                mTotalChunks.erase(it);
                return true;
            }else it++;
        }

        //If the chunk was not in the list it might still be pending recipe load. Check that.
        AV_INFO("Destroying chunk {}", coord);
        int targetRecipe = _recipeLoaded(coord);
        if(targetRecipe != -1){
            _activationList[targetRecipe] = false;
            _constructionList[targetRecipe] = false;
            return true;
        }

        return false;
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
                        //Construction is part of activation. So this can be reset.
                        _constructionList[i] = false;
                    }else if(_constructionList[i]){
                        //The chunk might just want construction.
                        _constructChunk(i);
                    }
                }

                traversalAmmount--;
                //If all the recipies which need updating have been processed then we can break.
                if(traversalAmmount <= 0) break;
            }
        }
    }

    bool SlotManager::deActivateChunk(const ChunkCoordinate &coord){
        //Check the current chunks.
        Chunk* chunk = _findChunk(coord);
        if(chunk){
            chunk->deActivate();
            return true;
        }
        //The chunk hasn't been constructed, so check the construction list.
        int chunkInActivation = _chunkInActivationList(coord);
        if(chunkInActivation != -1){
            //The chunk is set to be activated, but this has not yet happened. In this case just remove the activation request.
            _activationList[chunkInActivation] = false;
            //Activate was still called though, and all deactivate does is 'hide' the chunk.
            //So if the chunk is no longer going to be activated it still needs to be constructed.
            _constructionList[chunkInActivation] = true;
            return true;
        }
        return false;
    }

    bool SlotManager::constructChunk(const ChunkCoordinate &coord){
        if(_findChunk(coord)){
            AV_WARN("The chunk {} is already constructed.", coord);
            return false;
        }

        int targetRecipe = _recipeLoaded(coord);
        if(targetRecipe != -1){
            //The recipe is loaded.
            //Check if the recipe is ready.
            if(_recipeContainer[targetRecipe].recipeReady)
                _constructChunk(targetRecipe);
            else{
                //If the chunk isn't ready, add it to the activation list to be processed later.
                ///It should already be in the processing list.
                _constructionList[targetRecipe] = true;
                _updateNeededCount++;
            }
        }else{
            //The recipe is not loaded.
            //Load it.
            int recipeIndex = _loadRecipe(coord);

            //The load job has now started.
            //Add the recipe to the activation list.
            _constructionList[recipeIndex] = true;
            _updateNeededCount++;
        }
        return true;
    }

    bool SlotManager::setCurrentMap(const Ogre::String& map){
        if(map == WorldSingleton::mCurrentMap) return false;

        WorldEventMapChange event;
        event.oldMapName = WorldSingleton::mCurrentMap;
        event.newMapName = map;

        WorldSingleton::mCurrentMap = map;

        EventDispatcher::transmitEvent(EventType::World, event);

        return true;
    }

    bool SlotManager::setOrigin(const SlotPosition &pos){
        if(pos == WorldSingleton::getOrigin()) return false;

        WorldEventOriginChange event;
        event.oldPos = WorldSingleton::_origin;
        event.newPos = pos;

        WorldSingleton::_origin = pos;

        EventDispatcher::transmitEvent(EventType::World, event);

        _repositionChunks();

        return true;
    }

    void SlotManager::_repositionChunks(){
        for(ChunkEntry& e : mTotalChunks){
            AV_INFO(e.first)
            e.second->reposition();
        }
    }

    Chunk* SlotManager::_constructChunk(int recipe, bool positionChunk){
        Chunk *chunk = _findChunk(_recipeContainer[recipe].coord);
        if(!chunk){
            //The chunk does not exist and needs to be created.
            chunk = mChunkFactory->constructChunk(_recipeContainer[recipe], positionChunk);
            mTotalChunks.push_back(std::pair<ChunkCoordinate, Chunk*>(_recipeContainer[recipe].coord, chunk));
        }

        return chunk;
    }

    Chunk* SlotManager::_findChunk(const ChunkCoordinate &coord) const{
        for(const ChunkEntry &pair : mTotalChunks){
            if(pair.first == coord) return pair.second;
        }
        return 0;
    }

    void SlotManager::_activateChunk(int recipe){
        AV_INFO("Activating chunk {}", _recipeContainer[recipe].coord);

        Chunk* chunk = _constructChunk(recipe, true);

        chunk->reposition();
        chunk->activate();
    }

    int SlotManager::_chunkInActivationList(const ChunkCoordinate &coord){
        for(int i = 0; i < _MaxRecipies; i++){
            if(_activationList[i]){
                if(_recipeContainer[i].coord == coord) return i;
            }
        }
        return -1;
    }

    int SlotManager::_loadRecipe(const ChunkCoordinate &coord){
        //Get a position in the array.
        int targetIndex = _claimRecipeEntry();

        _recipeContainer[targetIndex].coord = coord;

        //_recipeContainer[targetIndex].ogreMeshData = new std::vector<int>();
        JobDispatcher::dispatchJob(new RecipeOgreMeshJob(&_recipeContainer[targetIndex]));

        _processingList[targetIndex] = true;
        _updateNeededCount++;

        return targetIndex;
    }

    int SlotManager::_recipeLoaded(const ChunkCoordinate &coord){
        for(int i = 0; i < _MaxRecipies; i++){
            //If the slot is available then there's nothing in it, so don't bother checking.
            if(_recipeContainer[i].slotAvailable) continue;

            if(coord == _recipeContainer[i].coord) return i;
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
        //Not sure if this is the most efficient.
        //The idea is to save the ammount of time taken to find the next recipe entry.
        mNextBlankRecipe = _findNextBlank(targetIndex);

        //reset the values in that recipe.
        _recipeContainer[targetIndex].recipeScore = 0;
        _recipeContainer[targetIndex].slotAvailable = false;
        _recipeContainer[targetIndex].coord = ChunkCoordinate();
        _recipeContainer[targetIndex].recipeReady = false;

        if(_recipeContainer[targetIndex].ogreMeshData)
            delete _recipeContainer[targetIndex].ogreMeshData;

        _recipeContainer[targetIndex].ogreMeshData = 0;
        _recipeContainer[targetIndex].jobDoneCounter = 0;


        _processingList[targetIndex] = false;
        _activationList[targetIndex] = false;

        return targetIndex;
    }

    int SlotManager::_obtainRecipeEntry(){
        //int retPos = mNextBlankRecipe;
        //mNextBlankRecipe = _findNextBlank(retPos);
        int retPos = _findNextBlank(mNextBlankRecipe);
        //TODO mNextBlankRecipe needs to be incremented somewhere. Decide where that should be.

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

    int SlotManager::_findNextBlank(int start) const{
        //Don't check -1.
        if(start == -1) return start;

        //Iterate the rest of the array.
        for(int i = 0; i < _MaxRecipies - start; i++){
            int index = i+start;
            if(_recipeContainer[index].slotAvailable) return index;
        }
        //If no next place can be found return this to specify that.
        return -1;
    }
};
