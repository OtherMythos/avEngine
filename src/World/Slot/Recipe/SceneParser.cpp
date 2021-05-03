#include "SceneParser.h"

#include "System/Util/PathUtils.h"
#include "OgreStringConverter.h"

#include <fstream>

namespace AV{
    SceneParser::SceneParser(){

    }

    SceneParser::~SceneParser(){

    }

    void SceneParser::_clearRecipeData(RecipeData* recipeData) const{
        if(recipeData->sceneEntries) delete recipeData->sceneEntries;
        if(recipeData->ogreMeshData) delete recipeData->ogreMeshData;

        recipeData->sceneEntries = 0;
        recipeData->ogreMeshData = 0;
    }

    void SceneParser::_populateRecipeData(RecipeData* recipeData) const {
        recipeData->sceneEntries = new std::vector<RecipeSceneEntry>();
        recipeData->ogreMeshData = new std::vector<OgreMeshRecipeData>();
    }

    bool SceneParser::parse(const std::string& dirPath, RecipeData* data){
        _clearRecipeData(data);
        _populateRecipeData(data);

        if(!_parse(dirPath, data)){
            _clearRecipeData(data);
            return false;
        }
        return true;
    }

    bool SceneParser::_parse(const std::string& dirPath, RecipeData* data){
        unsigned int expectedStaticMeshCount = 0;
        if(!_parseSceneTreeFile(dirPath + "/sceneTree.txt", data, &expectedStaticMeshCount)){
            return false;
        }
        if(!_parseStaticMeshes(dirPath + "/staticMeshes.txt", data)){
            return false;
        }

        if(expectedStaticMeshCount != data->ogreMeshData->size()){
            mFailureReason = "Mismatch of meshes in the mesh file and the used meshes.";
            return false;
        }

        return true;
    }

    bool SceneParser::_parseStaticMeshes(const std::string& filePath, RecipeData* recipeData){
        std::ifstream file;
        file.open(filePath);
        if(!file.is_open()) {
            mFailureReason = "Error opening staticMesh file path: " + filePath;
            return false;
        }

        std::string line;
        while(_getLine(file, line)){
            //Entries should come in 2s.
            std::string materialName;
            _getLine(file, materialName);
            Ogre::IdString targetString;
            if(!materialName.empty()) targetString = materialName;
            recipeData->ogreMeshData->push_back({line, targetString});
        }

        return true;
    }

    bool SceneParser::_getLine(std::ifstream& file, std::string& line){
        getline(file, line);
        if(line.empty()){
            mFailureReason = "Error reading line from file.";
            return false;
        }

        return true;
    }

    bool SceneParser::_parseSceneTreeFile(const std::string& filePath, RecipeData* recipeData, unsigned int* expectedMeshes){

        std::ifstream file;
        file.open(filePath);
        if(!file.is_open()) {
            mFailureReason = "Error opening sceneTree file path: " + filePath;
            return false;
        }

        int childCount = 0;
        std::string line;
        while(_getLine(file, line)){
            HeaderData headerData;
            if(!_readHeaderLine(line, &headerData)) return false;

            RecipeSceneEntry entry;
            entry.type = headerData.type;
            if(headerData.type == SceneType::child) childCount++;
            if(headerData.type == SceneType::term) childCount--;

            //The type is not a terminator, so read the next lines from it.
            if(headerData.type == SceneType::staticPhysicsShape){
                //Physics shapes specify a single line for the shape type.
                if(!_getLine(file, line)) return false;
            }

            if(headerData.hasPosition){
                if(!_getLine(file, line)) return false;
                entry.pos = Ogre::StringConverter::parseVector3(line);
            }else entry.pos = Ogre::Vector3::ZERO;
            if(headerData.hasScale){
                if(!_getLine(file, line)) return false;
                entry.scale = Ogre::StringConverter::parseVector3(line);
            }else entry.scale = Ogre::Vector3::UNIT_SCALE;
            if(headerData.hasOrientation){
                if(!_getLine(file, line)) return false;
                entry.orientation = Ogre::StringConverter::parseQuaternion(line);
            }else entry.orientation = Ogre::Quaternion::IDENTITY;
            entry.id = 0; //For now

            if(entry.type == SceneType::mesh && expectedMeshes) (*expectedMeshes)++;

            recipeData->sceneEntries->push_back(entry);
        }
        if(childCount != 0){
            mFailureReason = "Mismatch of child and terminator characters in the scene tree.";
            return false;
        }

        return true;
    }

    bool SceneParser::_readHeaderLine(const std::string& line, HeaderData* data){
        //TODO Put a regex here. I just need to figure out the format of the entries.

        if(!_populateSceneType(line[0], &data->type)) return false;
        if(!_populateBool(line[2], &data->hasPosition)) return false;
        if(!_populateBool(line[4], &data->hasScale)) return false;
        if(!_populateBool(line[6], &data->hasOrientation)) return false;

        return true;
    }

    bool SceneParser::_populateSceneType(char c, SceneType* type){
        switch(c){
            case '0': *type = SceneType::empty; break;
            case '1': *type = SceneType::child; break;
            case '2': *type = SceneType::term; break;
            case '3': *type = SceneType::mesh; break;
            case '4': *type = SceneType::staticPhysicsShape; break;
            case '5': *type = SceneType::collisionSender; break;
            case '6': *type = SceneType::empty; break; //Nav markers.
            case '7': *type = SceneType::empty; break; //Data points.
            default: {
                mFailureReason = "An invalid scene object type was read.";
                return false;
            }
        }

        return true;
    }

    bool SceneParser::_populateBool(char c, bool* b){
        switch(c){
            case '0': *b = false; break;
            case '1': *b = true; break;
            default: {
                mFailureReason = "Error reading boolean value.";
                return false;
            }
        }

        return true;
    }
}
