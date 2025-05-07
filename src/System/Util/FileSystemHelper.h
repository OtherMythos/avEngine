#pragma once

#include <rapidjson/document.h>
#include <string>

namespace AV{

    /**
    Helper class to abstract parts of the filesystem.
    Certain platforms treat the filesystem differently, or have different requirements.
    Notably, android requires use of a specific api in order to read files from the apk.
    */
    class FileSystemHelper{
    public:
        FileSystemHelper() = delete;
        ~FileSystemHelper() = delete;

        static bool setupRapidJsonDocument(const std::string& filePath, rapidjson::Document* doc);
    };
}