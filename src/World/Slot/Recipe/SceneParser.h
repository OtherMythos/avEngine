#pragma once

#include <string>
#include "RecipeData.h"

namespace AV{
    struct RecipeData;

    /**
    A class responsible for parsing scene data within a specific chunk.
    */
    class SceneParser{
    public:
        SceneParser();
        ~SceneParser();

        bool parse(const std::string& dirPath, RecipeData* data);

        const std::string& getFailureReason() const { return mFailureReason; }

    private:
        struct HeaderData{
            bool hasPosition;
            bool hasScale;

            SceneType type;
        };

        bool _parseSceneTreeFile(const std::string& filePath, RecipeData* data, unsigned int* expectedMeshes = 0);
        bool _readHeaderLine(const std::string& line, HeaderData* data);
        bool _populateBool(char c, bool* b);
        bool _populateSceneType(char c, SceneType* type);
        void _clearRecipeData(RecipeData* recipeData) const;
        void _populateRecipeData(RecipeData* recipeData) const;

        bool _parseStaticMeshes(const std::string& filePath, RecipeData* recipeData);
        bool _parse(const std::string& dirPath, RecipeData* data);

        bool _getLine(std::ifstream& file, std::string& line);

        std::string mFailureReason = "";
    };
}
