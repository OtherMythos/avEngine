#include "RecipeCollisionObjectsJob.h"

#include "Logger/Log.h"

#include "World/Slot/Recipe/RecipeData.h"
#include "World/Slot/Recipe/CollisionObjectSceneParser.h"
#include "System/SystemSetup/SystemSettings.h"

namespace AV{
    RecipeCollisionObjectsJob::RecipeCollisionObjectsJob(RecipeData *data)
    : _data(data){

    }

    void RecipeCollisionObjectsJob::process(){
        AV_INFO("Starting physics collision objects job! {}", _data->coord);

        if(!SystemSettings::isMapsDirectoryViable()) {
            return;
        }

        std::string filePath = SystemSettings::getMapsDirectory() + "/" + _data->coord.getFilePath() + "/collisionObjects.txt";
        CollisionObjectSceneParser parser;
        if(!parser.parse(filePath, _data)){
            AV_ERROR("There was an error processing collision objects job {}.", _data->coord);
        }
    }

    void RecipeCollisionObjectsJob::finish(){
        AV_INFO("Finishing physics collision objects job! {}", _data->coord);

        _data->jobDoneCounter++;
    }
}
