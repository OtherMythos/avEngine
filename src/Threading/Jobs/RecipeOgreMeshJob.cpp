#include "RecipeOgreMeshJob.h"

#include "Logger/Log.h"

#include "World/Slot/Recipe/RecipeData.h"
#include "World/Slot/Recipe/OgreMeshRecipeData.h"
#include "System/SystemSetup/SystemSettings.h"
#include "OgreStringConverter.h"

#include <string>

namespace AV{
    RecipeOgreMeshJob::RecipeOgreMeshJob(RecipeData *data)
    : _data(data){

    }

    void RecipeOgreMeshJob::process(){
        AV_INFO("Wow starting ogre recipe job!");
        AV_INFO(_data->coord);

        _data->ogreMeshData = new std::vector<OgreMeshRecipeData>();

        _processFile();
    }

    void RecipeOgreMeshJob::finish(){
        AV_INFO("Wow finishing ogre recipe job!");

        for(int i = 0; i < _data->ogreMeshData->size(); i++){
            OgreMeshRecipeData data = (*_data->ogreMeshData)[i];
            AV_INFO(data.meshName);
            AV_INFO(data.pos);
            AV_INFO(data.scale);
        }

        _data->jobDoneCounter++;
    }

    void RecipeOgreMeshJob::_processFile(){
        std::string filePath = SystemSettings::getDataPath() + "/chunks/chunk.txt";

        //TODO make the vector the correct size from the start by reading in a value.
        std::string line;
        std::ifstream myfile(filePath);
        if (myfile.is_open()){
            OgreMeshRecipeData data;
            int count = 0;

            while(getline (myfile,line)){
                if(count == 0){
                    data.meshName = line;
                    count++;
                }
                else if(count == 1){
                    data.pos = Ogre::StringConverter::parseVector3(line);
                    count++;
                }
                else if(count == 2){
                    data.scale = Ogre::StringConverter::parseVector3(line);
                    _data->ogreMeshData->push_back(data);
                    count = 0;
                }
            }
        }

    }
}
