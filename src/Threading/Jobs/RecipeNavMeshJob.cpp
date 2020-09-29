#include "RecipeNavMeshJob.h"

#include "Logger/Log.h"
#include "World/Slot/Recipe/RecipeData.h"
#include "System/SystemSetup/SystemSettings.h"

#include "World/Nav/Parser/DetourMeshBinaryParser.h"
#include <string>

namespace AV{
    RecipeNavMeshJob::RecipeNavMeshJob(RecipeData *data)
    : mData(data){

    }

    void RecipeNavMeshJob::process(){
        //AV_INFO("Starting ogre recipe job! {}", mData->coord);

        _processFile();
    }

    void RecipeNavMeshJob::finish(){
        //AV_INFO("Finishing ogre recipe job! {}", mData->coord);

        mData->jobDoneCounter++;
    }

    bool RecipeNavMeshJob::_processFile(){
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if(!SystemSettings::isMapsDirectoryViable()) {
            AV_ERROR("There was an error processing nav mesh recipe job {}. The maps directory isn't viable.", mData->coord);
            return false;
        }

        //Temporary for now.
        const std::string filePath = SystemSettings::getMapsDirectory() + "/" + mData->coord.getFilePath() + "/nav/0.nav";
        DetourMeshBinaryParser parser;
        dtNavMesh* mesh = parser.parseFile(filePath);
        if(!mesh){
            AV_ERROR("Error parsing nav mesh.");
            mData->loadedNavMesh = 0;
            return false;
        }
        mData->loadedNavMesh = mesh;

        return true;
    }
}
