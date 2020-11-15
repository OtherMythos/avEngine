#include "RecipeDataPointJob.h"

#include "Logger/Log.h"
#include "World/Slot/Recipe/RecipeData.h"
#include <string>
#include "System/SystemSetup/SystemSettings.h"

#include "OgreStringConverter.h"
#include "System/Util/PathUtils.h"

namespace AV{
    RecipeDataPointJob::RecipeDataPointJob(RecipeData *data)
    : mData(data){

    }

    void RecipeDataPointJob::process(){
        _processFile();
    }

    void RecipeDataPointJob::finish(){
        mData->jobDoneCounter++;
    }

    bool RecipeDataPointJob::_processFile(){
        if(!SystemSettings::isMapsDirectoryViable()) {
            AV_ERROR("There was an error processing physics body job {}. The maps directory isn't viable.", mData->coord);
            return false;
        }

        std::string filePath = SystemSettings::getMapsDirectory() + "/" + mData->coord.getFilePath() + "/dataPoints.txt";

        std::string line;
        std::ifstream myfile(filePath);
        if (myfile.is_open()){
            //We can create the vector now as the file is valid.
            mData->dataPoints = new std::vector<DataPointEntry>();

            while(getline(myfile,line)){
                DataPointEntry dataPoint;
                dataPoint.pos = Ogre::StringConverter::parseVector3(line, Ogre::Vector3::ZERO);
                if(!getline(myfile, line)) break;

                Ogre::StringVector vec = Ogre::StringUtil::split(line);
                if(vec.size() != 3) return false;
                unsigned int type = Ogre::StringConverter::parseUnsignedInt(vec[0], 30000);
                unsigned int subType = Ogre::StringConverter::parseUnsignedInt(vec[1], 30000);
                int userValue = Ogre::StringConverter::parseUnsignedInt(vec[2]);
                if(type > 255 || subType > 255) return false;
                dataPoint.userData = userValue;
                dataPoint.type = static_cast<DataPointType>(type);
                dataPoint.subType = static_cast<DataPointSubType>(subType);

                mData->dataPoints->push_back(dataPoint);
            }
        }

        return true;
    }
}
