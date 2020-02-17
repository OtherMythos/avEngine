#include "SceneParser.h"

#include "System/Util/PathUtils.h"
#include "OgreStringConverter.h"

namespace AV{
    SceneParser::SceneParser(){

    }

    SceneParser::~SceneParser(){

    }

    bool SceneParser::parse(const std::string& dirPath, RecipeData* data){

        if(!_parseSceneTreeFile(dirPath + "/sceneTree.txt")){
            return false;
        }

        return true;
    }

    bool SceneParser::_parseSceneTreeFile(const std::string& filePath){

        std::ifstream file;
        file.open(filePath);
        if(!file.is_open()) {
            mFailureReason = "Error opening sceneTree file path: " + filePath;
            return false;
        }

        std::string line;
        while(getline(file, line)){
            HeaderData data;
            if(!_readHeaderLine(line, &data)) return false;

            if(data.type == HeaderData::child);

            //The type is not a terminator, so read the next lines from it.

            if(data.hasPosition){
                if(!getline(file, line)) return false;
                //TODO finish this off.
                //creationData.scale = Ogre::StringConverter::parseVector3(line);
            }

        }

        return true;
    }

    bool SceneParser::_readHeaderLine(const std::string& line, HeaderData* data) const {
        //TODO Put a regex here. I just need to figure out the format of the entries.

        HeaderData outData;
        if(!_populateSceneType(line[0], &outData.type)) return false;
        if(!_populateBool(line[2], &outData.hasPosition)) return false;
        if(!_populateBool(line[4], &outData.hasScale)) return false;

        return true;
    }

bool SceneParser::_populateSceneType(char c, HeaderData::SceneType* type) const{
        switch(c){
            case '0': *type = HeaderData::empty; break;
            case '1': *type = HeaderData::child; break;
            case '2': *type = HeaderData::term; break;
            case '3': *type = HeaderData::mesh; break;
            default: return false; break;
        }

        return true;
    }

    bool SceneParser::_populateBool(char c, bool* b) const{
        switch(c){
            case '0': *b = false; break;
            case '1': *b = true; break;
            default: return false;
        }

        return true;
    }
}
