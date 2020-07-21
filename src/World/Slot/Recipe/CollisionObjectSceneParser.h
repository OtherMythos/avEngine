#pragma once

#include <string>
#include "RecipeData.h"

namespace AV{
    class CollisionObjectSceneParser{
    public:
        CollisionObjectSceneParser();
        ~CollisionObjectSceneParser();

        bool parse(const std::string& filePath, RecipeData* data);
        bool parse(const std::string& filePath, CollisionWorldChunkData& data);

        const std::string& getFailureReason() const { return mFailureReason; }

    private:
        std::string mFailureReason = "";
        uint32 mCurrentLine = 0;

        void _clearRecipeData(CollisionWorldChunkData& data) const;
        void _populateRecipeData(CollisionWorldChunkData& data) const;

        bool _parse(const std::string& filePath, CollisionWorldChunkData& data);

        bool _getLine(std::ifstream& file, std::string& line);

        typedef bool(CollisionObjectSceneParser::*ParserStageFunction)(std::string& line, std::ifstream& file, CollisionWorldChunkData& data);
        ParserStageFunction currentParseStage = 0;

        bool _parseShapes(std::string& line, std::ifstream& file, CollisionWorldChunkData& data);
        bool _parseScripts(std::string& line, std::ifstream& file, CollisionWorldChunkData& data);
        bool _parseClosures(std::string& line, std::ifstream& file, CollisionWorldChunkData& data);
        bool _parseScriptClosurePairs(std::string& line, std::ifstream& file, CollisionWorldChunkData& data);
        bool _parseCollisionObjectData(std::string& line, std::ifstream& file, CollisionWorldChunkData& data);
        bool _parseCollisionObjectEntries(std::string& line, std::ifstream& file, CollisionWorldChunkData& data);
    };
}
