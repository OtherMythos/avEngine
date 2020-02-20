#include "RecipeOgreMeshJob.h"

#include "Logger/Log.h"

#include "World/Slot/Recipe/RecipeData.h"
#include "World/Slot/Recipe/OgreMeshRecipeData.h"
#include "System/SystemSetup/SystemSettings.h"
#include "OgreStringConverter.h"

#include "World/Slot/Recipe/SceneParser.h"

#include <string>

namespace AV{
    RecipeOgreMeshJob::RecipeOgreMeshJob(RecipeData *data)
    : mData(data){

    }

    void RecipeOgreMeshJob::process(){
        AV_INFO("Starting ogre recipe job! {}", mData->coord);

        _processFile();
    }

    void RecipeOgreMeshJob::finish(){
        AV_INFO("Finishing ogre recipe job! {}", mData->coord);

        mData->jobDoneCounter++;
    }

    bool RecipeOgreMeshJob::_processFile(){
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if(!SystemSettings::isMapsDirectoryViable()) {
            AV_ERROR("There was an error processing ogre recipe job {}. The maps directory isn't viable.", mData->coord);
            return false;
        }

        SceneParser parser;
        parser.parse(SystemSettings::getMapsDirectory() + "/" + mData->coord.getFilePath(), mData);

        /*std::string filePath = SystemSettings::getMapsDirectory() + "/" + _data->coord.getFilePath() + "/meshes.txt";
        //std::string filePath = SystemSettings::getMapsDirectory() + "/" + ChunkCoordinate(0, 0, "overworld").getFilePath() + "/meshes.txt";

        std::string line;
        std::ifstream myfile(filePath);
        if (myfile.is_open()){
            OgreMeshRecipeData data;
            int count = -1;

            //We can create the vector now as the file is valid.
            auto vec = new std::vector<OgreMeshRecipeData>();
            _data->ogreMeshData = vec;

            while(getline (myfile,line)){
                if(count == -1){
                    //For efficiency, include how many entries are going to be in the vector and reserve that much memory upfront.
                    int vectorSize = Ogre::StringConverter::parseInt(line);
                    vec->reserve(vectorSize);
                    count = 0;
                }
                else if(count == 0){
                    data.meshName = line;
                    count++;
                }
                else if(count == 1){
                    data.pos = Ogre::StringConverter::parseVector3(line);
                    count++;
                }
                else if(count == 2){
                    data.scale = Ogre::StringConverter::parseVector3(line);
                    vec->push_back(data);
                    count = 0;
                }
            }
            myfile.close();
        }else{
            AV_INFO("Could not open ogre mesh resource file for coordinate {}", _data->coord);
            return false;
        }*/

        return true;
    }
}
