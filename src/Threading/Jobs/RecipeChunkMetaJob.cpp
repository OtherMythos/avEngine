#include "RecipeChunkMetaJob.h"

#include "Logger/Log.h"
#include "World/Slot/Recipe/RecipeData.h"
#include <string>
#include "System/SystemSetup/SystemSettings.h"

#include "System/Util/PathUtils.h"
#include "rapidjson/document.h"
#include <rapidjson/filereadstream.h>
#include "rapidjson/filewritestream.h"
#include <rapidjson/error/en.h>

namespace AV{
    RecipeChunkMetaJob::RecipeChunkMetaJob(RecipeData *data)
    : mData(data){

    }

    void RecipeChunkMetaJob::process(){
        _processFile();
    }

    void RecipeChunkMetaJob::finish(){
        mData->jobDoneCounter++;
    }

    bool RecipeChunkMetaJob::_processFile(){
        //Read the json file and write the data.

        std::string filePath = SystemSettings::getMapsDirectory() + "/" + mData->coord.getFilePath() + "/chunkMeta.json";
        if(!fileExists(filePath)){
            return false;
        }

        FILE* fp = fopen(filePath.c_str(), "r"); // non-Windows use "r"
        char readBuffer[65536];
        rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
        rapidjson::Document d;
        d.ParseStream(is);
        fclose(fp);

        if(d.HasParseError()){
            AV_ERROR("Error parsing chunk meta file.");
            AV_ERROR(rapidjson::GetParseError_En(d.GetParseError()));
            return false;
        }

        //Begin parsing the file.
        using namespace rapidjson;

        Value::ConstMemberIterator itr;

        //Parse values.
        {
            itr = d.FindMember("terrainEnabled");
            if(itr != d.MemberEnd() && itr->value.IsBool()){
                mData->chunkSettings.terrainEnabled = itr->value.GetBool();
            }
        }

        return true;
    }
}
