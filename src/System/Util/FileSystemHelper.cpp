#include "FileSystemHelper.h"

#include "sds_fstream.h"
#include "sds_fstreamApk.h"
#include "Logger/Log.h"

#include <vector>

#include <rapidjson/filereadstream.h>
#include <rapidjson/error/en.h>

namespace AV{
    bool FileSystemHelper::setupRapidJsonDocument(const std::string& filePath, rapidjson::Document* doc){
        sds::fstreamApk inFile(filePath, sds::fstream::InputEnd);
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

        return doc->HasParseError();
    }
}