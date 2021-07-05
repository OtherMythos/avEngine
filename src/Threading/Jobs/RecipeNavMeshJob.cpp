#include "RecipeNavMeshJob.h"

#include "Logger/Log.h"
#include "World/Slot/Recipe/RecipeData.h"
#include "System/SystemSetup/SystemSettings.h"

#include "World/Nav/Parser/DetourMeshBinaryParser.h"
#include <string>

#include "System/Util/PathUtils.h"

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

        DetourMeshBinaryParser parser;
        const std::string jsonFilePath = SystemSettings::getMapsDirectory() + "/" + mData->coord.getFilePath() + "/nav/navMeshData.json";
        if(!fileExists(jsonFilePath)){
            return false;
        }

        //Create the vector here if we've got this far.
        if(!mData->navMeshData){
            mData->navMeshData = new std::vector<NavMeshConstructionData>();
        }

        std::vector<NavMeshConstructionData>& outData = *(mData->navMeshData);
        outData.clear();
        bool success = parser.parseJsonMetaFile(jsonFilePath, outData);
        if(!success) return false;

        //TODO temporary while I decide what to do about multiple meshes.
        const std::string basePath = SystemSettings::getMapsDirectory() + "/" + mData->coord.getFilePath() + "/nav/";
        for(size_t i = 0; i < outData.size() && i < 1; i++){
            //Temporary for now.
            const std::string filePath = basePath + std::to_string(i) + ".nav";
            dtNavMesh* mesh = parser.parseFile(filePath);
            if(!mesh){
                AV_ERROR("Error parsing nav mesh {} with name {}.", i, outData[i].meshName);
                mData->navMeshData = 0;
                return false;
            }
            outData[i].mesh = mesh;
        }

        //Return true here because it might've just not found anything rather than failed.
        if(outData.size() == 0) return true;



        return true;
    }
}
