#include "SlotManager.h"

#include "Logger/Log.h"

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

        _loadRecipe(coord, QueuedRecipeType::RecipeTypeNone);

        return true;
    }

    bool SlotManager::unloadChunk(const ChunkCoordinate &coord){
        return true;
    }

    bool SlotManager::activateChunk(const ChunkCoordinate &coord){
        return _handleChunkRequest(coord, true);
    }

    bool SlotManager::constructChunk(const ChunkCoordinate &coord){
        return _handleChunkRequest(coord, false);
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
            //There's no need to actually destroy the recipe, as it'll be destroyed when the space is needed.
            _activationList[targetRecipe] = false;
            _constructionList[targetRecipe] = false;
            return true;
        }

        return false;
    }

    void SlotManager::update(){
        //There are no recipies waiting for update, so don't bother updating.
        if(!_updateNeeded()) return;

        AV_INFO("Slot manager updating");

        for(int i = 0; i < _MaxRecipies; i++){
            if(_recipeProcessing(i)){
                //Check the progress.
                if(_recipeContainer[i].jobDoneCounter >= RecipeData::targetJobs){
                    //The recipe is finished processing.
                    _recipeContainer[i].recipeReady = true;
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
                        _constructionList[i] = false;
                    }

                    //If there's an entry in the queue replace this freshly completed entry with that, as after the construction its space is needed.
                    if(queuedEntries.size() > 0){
                        //TODO this could be improved by telling it which slot to load the recipe in.
                        //At the moment it would do a complete traversal which isn't the best.
                        QueueEntry entry = queuedEntries.front();
                        //No need to check the chance of a -1 here.
                        int target = _loadRecipe(entry.first, entry.second);
                        _constructionList[target] = false;
                        _activationList[target] = false;
                        if(entry.second == QueuedRecipeType::RecipeTypeConstruct) _constructionList[target] = true;
                        if(entry.second == QueuedRecipeType::RecipeTypeActivate) {
                            _constructionList[target] = true;
                            _activationList[target] = true;
                        }
                        queuedEntries.pop_front();
                        _updateNeededCount++;
                    }
                }
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

    bool SlotManager::_handleChunkRequest(const ChunkCoordinate &coord, bool activate){
        //First check whether the chunk is already constructed.
        Chunk *chunk = _findChunk(coord);
        if(chunk){
            //The chunk is loaded so deal with that.
            //If this is a construct request we don't need to bother constructing it again, but we do need to return.
            if(activate) chunk->activate();
            return true;
        }
        //If the chunk is not already constructed then see what to do there.
        int targetRecipe = _recipeLoaded(coord);
        if(targetRecipe != -1){
            //The recipe is loaded.
            //Check if the recipe is ready.
            if(_recipeContainer[targetRecipe].recipeReady)
                if(activate) _activateChunk(targetRecipe);
                else _constructChunk(targetRecipe);
            else{
                //If the chunk isn't ready, add it to a list to be processed later.
                //It should already be in the processing list.
                if(activate) _activationList[targetRecipe] = true;
                //You can't activate a chunk without constructing it, and construction will want this true anyway, so this should always be set to true.
                _constructionList[targetRecipe] = true;
                _updateNeededCount++;
            }
        }else{
            //The recipe is not loaded.

            QueuedRecipeType recipeType = activate ? QueuedRecipeType::RecipeTypeActivate : QueuedRecipeType::RecipeTypeConstruct;

            //Now check if the entry is within the queue.
            auto it = _requestInQueue(coord);
            if(it != queuedEntries.end()){
                //Make sure this entry contains the intended queue type.
                (*it).second = recipeType;

                return true;
            }

            //If the recipe is nowhere to be found then try and load it.
            int recipeIndex = _loadRecipe(coord, recipeType);

            if(recipeIndex != -1){
                //The load job has now started.
                //Add the recipe to the activation list.
                if(activate) _activationList[recipeIndex] = true;
                _constructionList[recipeIndex] = true;
                _updateNeededCount++;
            }else{
                //The request had to be queued.
                //Nothing has to happen here as it's already been queued by loadRecipe.
            }

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

    std::deque<SlotManager::QueueEntry>::iterator SlotManager::_requestInQueue(const ChunkCoordinate &coord){
        auto it = queuedEntries.begin();
        while(it != queuedEntries.end()){
            if((*it).first == coord) return it;
            it++;
        }
        return queuedEntries.end();
    }

    bool SlotManager::_updateNeeded() const{
        if(_updateNeededCount <= 0) return false;

        return true;
    }

    void SlotManager::_repositionChunks(){
        for(ChunkEntry& e : mTotalChunks){
            AV_INFO(e.first)
            e.second->reposition();
        }
        mChunkFactory->reposition();
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

        Chunk* chunk = _constructChunk(recipe, false);

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

    bool SlotManager::_recipeProcessing(int loc){
        return !_recipeContainer[loc].recipeReady && !_recipeContainer[loc].slotAvailable;
    }

    int SlotManager::_loadRecipe(const ChunkCoordinate &coord, QueuedRecipeType loadType){
        //Get a position in the array.
        int targetIndex = _claimRecipeEntry();

        if(targetIndex != -1){
            _recipeContainer[targetIndex].coord = coord;

            mChunkFactory->startRecipeJob(&_recipeContainer[targetIndex]);
        }else{
            //There are no available recipe slots at the moment. This means we need to add the request to the queue.
            queuedEntries.push_back(QueueEntry(coord, loadType));
        }

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

        if(targetIndex != -1)
            _clearRecipeEntry(targetIndex);

        return targetIndex;
    }

    void SlotManager::_clearRecipeEntry(int targetIndex){
        //reset the values in that recipe.
        _recipeContainer[targetIndex].recipeScore = 0;
        _recipeContainer[targetIndex].slotAvailable = false;
        _recipeContainer[targetIndex].coord = ChunkCoordinate();
        _recipeContainer[targetIndex].recipeReady = false;

        if(_recipeContainer[targetIndex].ogreMeshData)
            delete _recipeContainer[targetIndex].ogreMeshData;

        _recipeContainer[targetIndex].ogreMeshData = 0;
        _recipeContainer[targetIndex].jobDoneCounter = 0;

        _activationList[targetIndex] = false;
        _constructionList[targetIndex] = false;
    }

    int SlotManager::_obtainRecipeEntry(){
        //int retPos = mNextBlankRecipe;
        //mNextBlankRecipe = _findNextBlank(retPos);
        int retPos = _findNextBlank(mNextBlankRecipe);
        //TODO mNextBlankRecipe needs to be incremented somewhere. Decide where that should be.

        //Here I need a function which either returns -1 or an index.
        //If it's minus one that means without a doubt that no recipies can fit at the moment.
        //Otherwise do something else.

        if(retPos == -1){
            //No recipe could be found. This means we need to make room.
            //int removalIndex = _findHighestScoringRecipe();
            int removalIndex = _determineReplacementIndex();

            retPos = removalIndex;
        }

        return retPos;
    }

    int SlotManager::_determineReplacementIndex(){
        //Find the highest scoring replacable index
        //Iterate all recipies.
        //If they're replacable then do the highest scoring check.
        //If no recipies are usable return -1.

        int highestIndex = -1;
        //HighestScore used to be 0. However, I can see that if all entries have a score of 0 that would mean that -1 might be returned when it shouldn't be.
        //Say its checking the third entry and this is the only entry not pending, it will perform a > operator on this entry.
        //If the value is 0 the > operator would have no effect and -1 would later be returned.
        //I suppose to start I should have the highest score as -1 so anything not pending becomes the first highest score.
        int highestScore = -1;

        for(int i = 0; i < _MaxRecipies; i++){
            //Chunk is loading and has threads working on it.
            if(_recipeProcessing(i)) continue;

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
