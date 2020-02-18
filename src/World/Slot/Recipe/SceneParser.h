#pragma once

#include <string>
#include "RecipeData.h"

namespace AV{
    class RecipeDataNew;

    /**
    A class responsible for parsing scene data within a specific chunk.
    */
    class SceneParser{
    public:
        SceneParser();
        ~SceneParser();

        bool parse(const std::string& dirPath, RecipeDataNew* data);

        const std::string& getFailureReason() const { return mFailureReason; }

    private:
        struct HeaderData{
            bool hasPosition;
            bool hasScale;

            SceneType type;
        };

        bool _parseSceneTreeFile(const std::string& filePath, RecipeDataNew* data, unsigned int* expectedMeshes = 0);
        bool _readHeaderLine(const std::string& line, HeaderData* data) const;
        bool _populateBool(char c, bool* b) const;
        bool _populateSceneType(char c, SceneType* type) const;
        void _clearRecipeData(RecipeDataNew* recipeData) const;
        void _populateRecipeData(RecipeDataNew* recipeData) const;

        bool _parseStaticMeshes(const std::string& filePath, RecipeDataNew* recipeData);

        std::string mFailureReason = "";
    };
}
