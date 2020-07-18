#include "CollisionObjectSceneParser.h"

#include <regex>
#include "OgreStringConverter.h"
#include "World/Physics/Worlds/CollisionWorldUtils.h"

#define GET_LINE_CHECK_TERMINATOR(x, z) \
    if(!_getLine(x, z)) return false; \
    if(z == "=="){ \
        mFailureReason = "Terminator line incorrectly placed."; \
        return false; \
    }

namespace AV{
    CollisionObjectSceneParser::CollisionObjectSceneParser(){

    }

    CollisionObjectSceneParser::~CollisionObjectSceneParser(){

    }

    void CollisionObjectSceneParser::_clearRecipeData(RecipeData* recipeData) const{
        if(recipeData->collisionObjectPackedData) delete recipeData->collisionObjectPackedData;
        if(recipeData->collisionShapeData) delete recipeData->collisionShapeData;
        if(recipeData->collisionScriptAndClosures) delete recipeData->collisionScriptAndClosures;
        if(recipeData->collisionScriptData) delete recipeData->collisionScriptData;
        if(recipeData->collisionObjectRecipeData) delete recipeData->collisionObjectRecipeData;

        recipeData->collisionObjectPackedData = 0;
        recipeData->collisionShapeData = 0;
        recipeData->collisionScriptAndClosures = 0;
        recipeData->collisionScriptData = 0;
        recipeData->collisionObjectRecipeData = 0;
    }

    void CollisionObjectSceneParser::_populateRecipeData(RecipeData* recipeData) const{
        recipeData->collisionObjectPackedData = new std::vector<int>();
        recipeData->collisionShapeData = new std::vector<PhysicsShapeRecipeData>();
        recipeData->collisionScriptAndClosures = new std::vector<std::string>();
        recipeData->collisionScriptData = new std::vector<CollisionObjectScriptData>();
        recipeData->collisionObjectRecipeData = new std::vector<CollisionObjectRecipeData>();
    }

    bool CollisionObjectSceneParser::parse(const std::string& filePath, RecipeData* data){
        _clearRecipeData(data);
        _populateRecipeData(data);

        if(!_parse(filePath, data)){
            _clearRecipeData(data);
            return false;
        }
        return true;
    }

    bool CollisionObjectSceneParser::_parse(const std::string& filePath, RecipeData* data){
        std::ifstream file;
        file.open(filePath);
        if(!file.is_open()) {
            mFailureReason = "Error opening collision objects file path: " + filePath;
            return false;
        }
        currentParseStage = &CollisionObjectSceneParser::_parseShapes;

        std::string line;
        while(_getLine(file, line)){
            bool result = (this->*currentParseStage)(line, file, data);
            if(!result) return false;
        }

        return true;
    }

    bool CollisionObjectSceneParser::_getLine(std::ifstream& file, std::string& line){
        getline(file, line);
        if(line.empty()){
            mFailureReason = "Error reading line from file.";
            return false;
        }

        return true;
    }

    bool CollisionObjectSceneParser::_parseShapes(std::string& line, std::ifstream& file, RecipeData* data){
        if(line == "=="){
            currentParseStage = &CollisionObjectSceneParser::_parseScripts;
            return true;
        }

        static const std::regex shapeTypeRegex("^\\d$");
        if(!std::regex_match(line, shapeTypeRegex)) return false;
        int shapeType = Ogre::StringConverter::parseInt(line);

        GET_LINE_CHECK_TERMINATOR(file, line);
        Ogre::Vector3 shapeScale = Ogre::StringConverter::parseVector3(line);

        assert(data->collisionShapeData);
        data->collisionShapeData->push_back({shapeType, OGRE_TO_BULLET(shapeScale)});

        return true;
    }

    bool CollisionObjectSceneParser::_parseScripts(std::string& line, std::ifstream& file, RecipeData* data){
        if(line == "=="){
            currentParseStage = &CollisionObjectSceneParser::_parseClosures;
            data->collisionClosuresBegin = data->collisionScriptAndClosures->size();
            return true;
        }

        data->collisionScriptAndClosures->push_back(line);

        return true;
    }

    bool CollisionObjectSceneParser::_parseClosures(std::string& line, std::ifstream& file, RecipeData* data){
        if(line == "=="){
            currentParseStage = &CollisionObjectSceneParser::_parseScriptClosurePairs;
            return true;
        }

        data->collisionScriptAndClosures->push_back(line);

        return true;
    }

    bool CollisionObjectSceneParser::_parseScriptClosurePairs(std::string& line, std::ifstream& file, RecipeData* data){
        if(line == "=="){
            currentParseStage = &CollisionObjectSceneParser::_parseCollisionObjectData;
            return true;
        }
        if(!std::regex_match(line, std::regex("^\\d* \\d*$"))) return false;

        size_t spacePos = line.find(" ");
        uint16 scriptIdx = Ogre::StringConverter::parseInt(line.substr(0, spacePos));
        uint16 closureIdx = Ogre::StringConverter::parseInt(line.substr(spacePos+1, line.size()));

        data->collisionScriptData->push_back({scriptIdx, closureIdx});

        return true;
    }

    bool CollisionObjectSceneParser::_parseCollisionObjectData(std::string& line, std::ifstream& file, RecipeData* data){
        if(line == "=="){
            currentParseStage = &CollisionObjectSceneParser::_parseCollisionObjectEntries;
            return true;
        }

        if(!std::regex_match(line, std::regex("^\\d$"))) return false;
        int worldId = Ogre::StringConverter::parseInt(line);
        if(worldId >= MAX_COLLISION_WORLDS){
            mFailureReason = "Invalid collision world.";
            return false;
        }

        GET_LINE_CHECK_TERMINATOR(file, line);

        if(!std::regex_match(line, std::regex("^(0|1){7}$"))) return false;

        char targetVals = 0;
        static const CollisionObjectTypeMask::CollisionObjectTypeMask targetMaskVals[7] = {
            CollisionObjectTypeMask::PLAYER,
            CollisionObjectTypeMask::ENEMY,
            CollisionObjectTypeMask::OBJECT,
            CollisionObjectTypeMask::USER_3,
            CollisionObjectTypeMask::USER_4,
            CollisionObjectTypeMask::USER_5,
            CollisionObjectTypeMask::USER_6
        };
        for(uint8 i = 0; i < 7; i++){
            if(line[i] == '1')
                targetVals |= targetMaskVals[i];
        }

        GET_LINE_CHECK_TERMINATOR(file, line);
        char eventVals = 0;
        static const CollisionObjectEventMask::CollisionObjectEventMask eventMaskVals[3] = {
            CollisionObjectEventMask::ENTER,
            CollisionObjectEventMask::LEAVE,
            CollisionObjectEventMask::INSIDE,
        };
        if(!std::regex_match(line, std::regex("^(0|1){3}$"))) return false;
        for(uint8 i = 0; i < 3; i++){
            if(line[i] == '1')
                eventVals |= eventMaskVals[i];
        }

        GET_LINE_CHECK_TERMINATOR(file, line);
        if(!std::regex_match(line, std::regex("^\\d*$"))) return false;
        int objectId = Ogre::StringConverter::parseInt(line);

        CollisionPackedInt packedInt = CollisionWorldUtils::producePackedInt(CollisionObjectType::SENDER_SCRIPT, worldId, targetVals, eventVals);

        data->collisionObjectPackedData->push_back(packedInt);

        return true;
    }

    bool CollisionObjectSceneParser::_parseCollisionObjectEntries(std::string& line, std::ifstream& file, RecipeData* data){

        uint16 shapeId = Ogre::StringConverter::parseInt(line);
        GET_LINE_CHECK_TERMINATOR(file, line);
        uint16 scriptId = Ogre::StringConverter::parseInt(line);
        GET_LINE_CHECK_TERMINATOR(file, line);
        uint16 dataId = Ogre::StringConverter::parseInt(line);
        GET_LINE_CHECK_TERMINATOR(file, line);

        Ogre::Vector3 pos = Ogre::StringConverter::parseVector3(line);
        GET_LINE_CHECK_TERMINATOR(file, line);
        Ogre::Quaternion orientation = Ogre::StringConverter::parseQuaternion(line);

        data->collisionObjectRecipeData->push_back({shapeId, scriptId, dataId, OGRE_TO_BULLET(pos), OGRE_TO_BULLET_QUAT(orientation)});

        return true;
    }
}
