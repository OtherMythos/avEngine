#pragma once

#include <string>

namespace AV{
    class RecipeData;

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

            enum SceneType{
                child,
                empty,
                term,
                mesh
            };
            SceneType type;
        };

        bool _parseSceneTreeFile(const std::string& filePath);
        bool _readHeaderLine(const std::string& line, HeaderData* data) const;
        bool _populateBool(char c, bool* b) const;
        bool _populateSceneType(char c, HeaderData::SceneType* type) const;

        std::string mFailureReason = "";
    };
}
