#include "CollisionObjectSceneParser.h"

#include <regex>
#include "OgreStringConverter.h"
#include "World/Physics/Worlds/CollisionWorldUtils.h"

#include <fstream>

#define GET_LINE_CHECK_TERMINATOR(x, z) \
    if(!_getLine(x, z)) return false; \
    if(z == "=="){ \
        mFailureReason = "Terminator line incorrectly placed."; \
        return false; \
    }

#define REGEX_CHECK(zz) \
    if(!std::regex_match(line, std::regex(zz))){ \
        mFailureReason = std::string("Regex failed: ") + zz; \
        return false; \
    }

namespace AV{
    CollisionObjectSceneParser::CollisionObjectSceneParser(){

    }

    CollisionObjectSceneParser::~CollisionObjectSceneParser(){

    }

    void CollisionObjectSceneParser::_clearRecipeData(CollisionWorldChunkData& data){
        if(data.collisionObjectPackedData) delete data.collisionObjectPackedData;
        if(data.collisionShapeData) delete data.collisionShapeData;
        if(data.collisionScriptAndClosures) delete data.collisionScriptAndClosures;
        if(data.collisionScriptData) delete data.collisionScriptData;
        if(data.collisionObjectRecipeData) delete data.collisionObjectRecipeData;

        data.collisionObjectPackedData = 0;
        data.collisionShapeData = 0;
        data.collisionScriptAndClosures = 0;
        data.collisionScriptData = 0;
        data.collisionObjectRecipeData = 0;
    }

    void CollisionObjectSceneParser::_populateRecipeData(CollisionWorldChunkData& data){
        data.collisionObjectPackedData = new std::vector<CollisionObjectPropertiesData>();
        data.collisionShapeData = new std::vector<PhysicsShapeRecipeData>();
        data.collisionScriptAndClosures = new std::vector<std::string>();
        data.collisionScriptData = new std::vector<CollisionObjectScriptData>();
        data.collisionObjectRecipeData = new std::vector<CollisionObjectRecipeData>();
    }

    bool CollisionObjectSceneParser::parse(const std::string& filePath, RecipeData* recipeData){
        CollisionWorldChunkData& data = recipeData->collisionData;
        return parse(filePath, data);
    }

    bool CollisionObjectSceneParser::parse(const std::string& filePath, CollisionWorldChunkData& data){
        _clearRecipeData(data);
        _populateRecipeData(data);

        if(!_parse(filePath, data)){
            _clearRecipeData(data);
            return false;
        }
        return true;
    }

    bool CollisionObjectSceneParser::_parse(const std::string& filePath, CollisionWorldChunkData& data){
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
        mCurrentLine++;
        if(!file.good()){
            mFailureReason = "Error reading line from file. Line ";
            mFailureReason.append(std::to_string(mCurrentLine));
            return false;
        }

        return true;
    }

    bool CollisionObjectSceneParser::_parseShapes(std::string& line, std::ifstream& file, CollisionWorldChunkData& data){
        if(line == "=="){
            currentParseStage = &CollisionObjectSceneParser::_parseScripts;
            return true;
        }

        REGEX_CHECK("^\\d$");
        int shapeType = Ogre::StringConverter::parseInt(line);

        GET_LINE_CHECK_TERMINATOR(file, line);
        Ogre::Vector3 shapeScale = Ogre::StringConverter::parseVector3(line);

        assert(data.collisionShapeData);
        data.collisionShapeData->push_back({shapeType, OGRE_TO_BULLET(shapeScale)});

        return true;
    }

    bool CollisionObjectSceneParser::_parseScripts(std::string& line, std::ifstream& file, CollisionWorldChunkData& data){
        if(line == "=="){
            currentParseStage = &CollisionObjectSceneParser::_parseClosures;
            data.collisionClosuresBegin = data.collisionScriptAndClosures->size();
            return true;
        }

        data.collisionScriptAndClosures->push_back(line);

        return true;
    }

    bool CollisionObjectSceneParser::_parseClosures(std::string& line, std::ifstream& file, CollisionWorldChunkData& data){
        if(line == "=="){
            currentParseStage = &CollisionObjectSceneParser::_parseScriptClosurePairs;
            return true;
        }

        data.collisionScriptAndClosures->push_back(line);

        return true;
    }

    bool CollisionObjectSceneParser::_parseScriptClosurePairs(std::string& line, std::ifstream& file, CollisionWorldChunkData& data){
        if(line == "=="){
            currentParseStage = &CollisionObjectSceneParser::_parseCollisionObjectData;
            return true;
        }
        REGEX_CHECK("^\\d+ \\d+$");

        size_t spacePos = line.find(" ");
        uint16 scriptIdx = Ogre::StringConverter::parseInt(line.substr(0, spacePos));
        uint16 closureIdx = Ogre::StringConverter::parseInt(line.substr(spacePos+1, line.size()));

        data.collisionScriptData->push_back({scriptIdx, closureIdx});

        return true;
    }

    bool CollisionObjectSceneParser::_parseCollisionObjectData(std::string& line, std::ifstream& file, CollisionWorldChunkData& data){
        if(line == "=="){
            currentParseStage = &CollisionObjectSceneParser::_parseCollisionObjectEntries;
            return true;
        }

        REGEX_CHECK("^\\d$");
        int worldId = Ogre::StringConverter::parseInt(line);
        if(worldId >= MAX_COLLISION_WORLDS){
            mFailureReason = "Invalid collision world.";
            return false;
        }

        GET_LINE_CHECK_TERMINATOR(file, line);

        REGEX_CHECK("^(0|1){7}$");

        char targetVals = 0;
        for(uint8 i = 0; i < 7; i++){
            if(line[i] == '1')
                targetVals |= (1u << i);
        }

        GET_LINE_CHECK_TERMINATOR(file, line);
        char eventVals = 0;
        REGEX_CHECK("^(0|1){3}$");
        for(uint8 i = 0; i < 3; i++){
            if(line[i] == '1')
                eventVals |= (1u << i);
        }

        GET_LINE_CHECK_TERMINATOR(file, line);
        REGEX_CHECK("^-?\\d+$");
        int objectId = Ogre::StringConverter::parseInt(line);

        CollisionPackedInt packedInt = CollisionWorldUtils::producePackedInt(CollisionObjectType::SENDER_SCRIPT, worldId, targetVals, eventVals);

        data.collisionObjectPackedData->push_back({packedInt, objectId});

        return true;
    }

    bool CollisionObjectSceneParser::_parseCollisionObjectEntries(std::string& line, std::ifstream& file, CollisionWorldChunkData& data){
        if(line.empty()){
            //Check for the end of the file. If the line is empty at the beginning we've found it.
            return true;
        }

        uint16 shapeId = Ogre::StringConverter::parseInt(line);
        GET_LINE_CHECK_TERMINATOR(file, line);
        uint16 scriptId = Ogre::StringConverter::parseInt(line);
        GET_LINE_CHECK_TERMINATOR(file, line);
        uint16 dataId = Ogre::StringConverter::parseInt(line);
        GET_LINE_CHECK_TERMINATOR(file, line);

        Ogre::Vector3 pos = Ogre::StringConverter::parseVector3(line);
        GET_LINE_CHECK_TERMINATOR(file, line);
        Ogre::Quaternion orientation = Ogre::StringConverter::parseQuaternion(line);

        data.collisionObjectRecipeData->push_back({shapeId, scriptId, dataId, OGRE_TO_BULLET(pos), OGRE_TO_BULLET_QUAT(orientation)});

        return true;
    }
}
