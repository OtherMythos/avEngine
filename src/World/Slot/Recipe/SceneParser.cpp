#include "SceneParser.h"

#include "System/Util/PathUtils.h"
#include "OgreStringConverter.h"

namespace AV{
    SceneParser::SceneParser(){

    }

    SceneParser::~SceneParser(){

    }

    void SceneParser::_clearRecipeData(RecipeDataNew* recipeData) const{
        if(recipeData->sceneEntries) delete recipeData->sceneEntries;
        if(recipeData->ogreMeshData) delete recipeData->ogreMeshData;

        recipeData->sceneEntries = 0;
        recipeData->ogreMeshData = 0;
    }

    void SceneParser::_populateRecipeData(RecipeDataNew* recipeData) const {
        recipeData->sceneEntries = new std::vector<RecipeSceneEntry>();
        recipeData->ogreMeshData = new std::vector<OgreMeshRecipeDataNew>();
    }

    bool SceneParser::parse(const std::string& dirPath, RecipeDataNew* data){

        _clearRecipeData(data);
        _populateRecipeData(data);

        if(!_parseSceneTreeFile(dirPath + "/sceneTree.txt", data)){
            return false;
        }

        return true;
    }

    bool SceneParser::_parseSceneTreeFile(const std::string& filePath, RecipeDataNew* recipeData){

        std::ifstream file;
        file.open(filePath);
        if(!file.is_open()) {
            mFailureReason = "Error opening sceneTree file path: " + filePath;
            return false;
        }

        std::string line;
        while(getline(file, line)){
            HeaderData headerData;
            if(!_readHeaderLine(line, &headerData)) return false;

            RecipeSceneEntry entry;
            entry.type = headerData.type;
            if(headerData.type == SceneType::child);

            //The type is not a terminator, so read the next lines from it.

            if(headerData.hasPosition){
                if(!getline(file, line)) return false;
                entry.pos = Ogre::StringConverter::parseVector3(line);
            }else entry.pos = Ogre::Vector3::ZERO;
            if(headerData.hasScale){
                if(!getline(file, line)) return false;
                entry.scale = Ogre::StringConverter::parseVector3(line);
            }else entry.scale = Ogre::Vector3::ZERO;
            entry.id = 0; //For now

            recipeData->sceneEntries->push_back(entry);
        }

        return true;
    }

    bool SceneParser::_readHeaderLine(const std::string& line, HeaderData* data) const {
        //TODO Put a regex here. I just need to figure out the format of the entries.

        if(!_populateSceneType(line[0], &data->type)) return false;
        if(!_populateBool(line[2], &data->hasPosition)) return false;
        if(!_populateBool(line[4], &data->hasScale)) return false;

        return true;
    }

bool SceneParser::_populateSceneType(char c, SceneType* type) const{
        switch(c){
            case '0': *type = SceneType::empty; break;
            case '1': *type = SceneType::child; break;
            case '2': *type = SceneType::term; break;
            case '3': *type = SceneType::mesh; break;
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
