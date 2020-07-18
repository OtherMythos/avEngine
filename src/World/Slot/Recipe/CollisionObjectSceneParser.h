#pragma once

#include <string>
#include "RecipeData.h"

namespace AV{
    class CollisionObjectSceneParser{
    public:
        CollisionObjectSceneParser();
        ~CollisionObjectSceneParser();

        bool parse(const std::string& filePath, RecipeData* data);

    private:
        std::string mFailureReason = "";

        void _clearRecipeData(RecipeData* recipeData) const;
        void _populateRecipeData(RecipeData* recipeData) const;

        bool _parse(const std::string& filePath, RecipeData* data);

        bool _getLine(std::ifstream& file, std::string& line);

        typedef bool(CollisionObjectSceneParser::*ParserStageFunction)(std::string& line, std::ifstream& file, RecipeData* data);
        ParserStageFunction currentParseStage = 0;

        bool _parseShapes(std::string& line, std::ifstream& file, RecipeData* data);
        bool _parseScripts(std::string& line, std::ifstream& file, RecipeData* data);
        bool _parseClosures(std::string& line, std::ifstream& file, RecipeData* data);
        bool _parseScriptClosurePairs(std::string& line, std::ifstream& file, RecipeData* data);
        bool _parseCollisionObjectData(std::string& line, std::ifstream& file, RecipeData* data);
        bool _parseCollisionObjectEntries(std::string& line, std::ifstream& file, RecipeData* data);
    };
}
