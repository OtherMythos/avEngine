#include "RecipeSceneJob.h"

#include "Logger/Log.h"

#include "System/SystemSetup/SystemSettings.h"
#include "World/Slot/Recipe/SceneParser.h"

#include <string>

namespace AV{
    RecipeSceneJob::RecipeSceneJob(RecipeData *data)
    : mData(data){

    }

    void RecipeSceneJob::process(){
        //AV_INFO("Starting ogre recipe job! {}", mData->coord);

        _processFile();
    }

    void RecipeSceneJob::finish(){
        //AV_INFO("Finishing ogre recipe job! {}", mData->coord);

        mData->jobDoneCounter++;
    }

    bool RecipeSceneJob::_processFile(){
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if(!SystemSettings::isMapsDirectoryViable()) {
            AV_ERROR("There was an error processing scene recipe job {}. The maps directory isn't viable.", mData->coord);
            return false;
        }

        SceneParser parser;
        bool result = parser.parse(SystemSettings::getMapsDirectory() + "/" + mData->coord.getFilePath(), mData);
        if(!result){
            AV_ERROR("There was an error processing scene recipe job {}.", mData->coord);
            AV_ERROR(parser.getFailureReason());
            return false;
        }

        return true;
    }
}
