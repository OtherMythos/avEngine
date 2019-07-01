#include "RecipePhysicsBodiesJob.h"

#include "Logger/Log.h"

#include "World/Slot/Recipe/RecipeData.h"
#include "World/Slot/Recipe/OgreMeshRecipeData.h"
#include "System/SystemSetup/SystemSettings.h"
#include "OgreStringConverter.h"

#include <string>

namespace AV{
    RecipePhysicsBodiesJob::RecipePhysicsBodiesJob(RecipeData *data)
    : _data(data){

    }

    void RecipePhysicsBodiesJob::process(){
        AV_INFO("Starting physics bodies job! {}", _data->coord);

        _processFile();
    }

    void RecipePhysicsBodiesJob::finish(){
        AV_INFO("Finishing physics bodies job! {}", _data->coord);

        _data->jobDoneCounter++;
    }

    bool RecipePhysicsBodiesJob::_processFile(){
        if(!SystemSettings::isMapsDirectoryViable()) {
            AV_ERROR("There was an error processing physics body job {}. The maps directory isn't viable.", _data->coord);
            return false;
        }

        std::string filePath = SystemSettings::getMapsDirectory() + "/" + _data->coord.getFilePath() + "/bodies.txt";

        std::string line;
        std::ifstream myfile(filePath);
        if (myfile.is_open()){

            int count = 0;

            //We can create the vector now as the file is valid.
            auto shapeVec = new std::vector<PhysicsShapeRecipeData>();
            auto bodyVec = new std::vector<PhysicsBodyRecipeData>();
            _data->physicsBodyData = bodyVec;
            _data->physicsShapeData = shapeVec;

            PhysicsBodyRecipeData bodyData;
            PhysicsShapeRecipeData shapeData;
            while(getline (myfile,line)){
                if(line == "=="){
                    //Start processing bodies.
                    count = 2;
                    continue;
                }
                //Processing shapes.
                if(count == 0){
                    shapeData.physicsShapeType = Ogre::StringConverter::parseInt(line);
                    count++;
                }
                else if(count == 1){
                    Ogre::Vector3 bounds = Ogre::StringConverter::parseVector3(line);
                    shapeData.scale = btVector3(bounds.x, bounds.y, bounds.z);
                    shapeVec->push_back(shapeData);
                    count = 0;
                }
                else if(count == 2){
                    bodyData.shapeId = Ogre::StringConverter::parseInt(line);
                    count++;
                }
                else if(count == 3){
                    Ogre::Vector3 pos = Ogre::StringConverter::parseVector3(line);
                    bodyData.pos = btVector3(pos.x, pos.y, pos.z);
                    count++;
                }
                else if(count == 4){
                    Ogre::Quaternion ori = Ogre::StringConverter::parseQuaternion(line);
                    bodyData.orientation = btQuaternion(ori.x, ori.y, ori.z, ori.w);
                    bodyVec->push_back(bodyData);
                    count = 2;
                }
            }
            myfile.close();
        }else{
            AV_INFO("Could not open the physics body resource file for coordinate {}", _data->coord);
            return false;
        }

        return true;
    }
}
