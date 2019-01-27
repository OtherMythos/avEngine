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
        AV_INFO("Starting ogre recipe job! {}", _data->coord);

        
        
        _processFile();
    }

    void RecipeOgreMeshJob::finish(){
        AV_INFO("Finishing ogre recipe job! {}", _data->coord);

        _data->jobDoneCounter++;
    }

    void RecipeOgreMeshJob::_processFile(){
        auto vec = new std::vector<OgreMeshRecipeData>();
        _data->ogreMeshData = vec;
        std::string filePath = SystemSettings::getDataPath() + "/chunks/chunk.txt";
        
        std::string line;
        std::ifstream myfile(filePath);
        if (myfile.is_open()){
            OgreMeshRecipeData data;
            int count = -1;
            
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
        }

    }
}
