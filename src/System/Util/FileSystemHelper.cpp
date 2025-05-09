#include "FileSystemHelper.h"

#include "sds_fstream.h"
#include "sds_fstreamApk.h"
#include "System/FileSystem/FilePath.h"
#include "Logger/Log.h"

#include "Ogre.h"
#include "tinyxml2.h"

#include <vector>

#include <rapidjson/filereadstream.h>
#include <rapidjson/error/en.h>

namespace AV{
    bool _processRapidJSONDocument(sds::fstreamApk& inFile, const std::string& filePath, rapidjson::Document* doc){
        if(!inFile.good()){
            AV_ERROR("Error opening JSON file at path '{}'", filePath);
            return false;
        }

        const size_t fileSize = inFile.getFileSize(false);
        inFile.seek(0, sds::fstream::beg);

        std::vector<char> fileData;
        fileData.resize(fileSize + 1u);
        inFile.read(&fileData[0], fileSize);
        fileData[fileSize] = '\0'; // Add null terminator

        doc->Parse(&fileData[0]);

        if(doc->HasParseError()){
            AV_ERROR("Error parsing file '{}'", rapidjson::GetParseError_En(doc->GetParseError()));
            return false;
        }

        return true;
    }
    bool FileSystemHelper::setupRapidJsonDocument(const std::string& filePath, rapidjson::Document* doc){
        AV::FilePath testPath(filePath);
        if(testPath.exists()){
            sds::fstreamApk inFile(filePath, sds::fstream::InputEnd);
            return _processRapidJSONDocument(inFile, filePath, doc);
        }else{
            sds::fstreamApk inFile(filePath, sds::fstream::InputEnd, false);
            return _processRapidJSONDocument(inFile, filePath, doc);
        }
    }

    bool FileSystemHelper::loadOgreConfigFile(Ogre::ConfigFile& cf, const std::string& path){
        #ifdef TARGET_ANDROID
            sds::fstreamApk inFile(path, sds::fstream::InputEnd);
            if(!inFile.good()){
                AV_ERROR("Error opening config file at path '{}'", path);
                return false;
            }

            size_t length = inFile.getFileSize(false);

            std::vector<char> fileData;
            fileData.resize(length + 1u);
            inFile.seek(0, sds::fstream::beg);
            inFile.read(&fileData[0], length);
            fileData[length] = '\0'; // Add null terminator

            Ogre::DataStreamPtr stream = Ogre::DataStreamPtr(
                new Ogre::MemoryDataStream(static_cast<void*>(&fileData[0]), length, false)
            );

            cf.load(stream);

            inFile.close();
        #else
            cf.load(path);
        #endif

        return true;
    }

    bool FileSystemHelper::loadXMLDocument(tinyxml2::XMLDocument& xmlDoc, const std::string& path){
        #ifdef TARGET_ANDROID
            sds::fstreamApk inFile(path, sds::fstream::InputEnd);
            if(!inFile.good()){
                AV_ERROR("Error opening XML file at path '{}'", path);
                return false;
            }

            size_t length = inFile.getFileSize(false);

            std::vector<char> fileData;
            fileData.resize(length + 1u);
            inFile.seek(0, sds::fstream::beg);
            inFile.read(&fileData[0], length);
            fileData[length] = '\0'; // Add null terminator

            tinyxml2::XMLError result = xmlDoc.Parse(&fileData[0], length);
        #else
            tinyxml2::XMLError result = xmlDoc.LoadFile(path.c_str());
        #endif

        if(result != tinyxml2::XML_SUCCESS) {
            AV_ERROR("Error when opening the animation file: {}", path);
            AV_ERROR("{}", xmlDoc.ErrorStr());
            return false;
        }

        return true;
    }
}