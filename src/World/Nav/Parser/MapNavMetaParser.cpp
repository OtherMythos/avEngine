#include "MapNavMetaParser.h"

#include <iostream>
#include <cstring>

#include "Logger/Log.h"

#include "System/Util/PathUtils.h"

#include "rapidjson/document.h"
#include <rapidjson/filereadstream.h>
#include <rapidjson/error/en.h>

namespace AV{
    MapNavMetaParser::MapNavMetaParser(){

    }

    bool MapNavMetaParser::parseFile(const std::string& filePath, std::vector<MapNavMetaParserData>& out){
        if(!fileExists(filePath)){
            return false;
        }

        out.clear();

        FILE* fp = fopen(filePath.c_str(), "r"); // non-Windows use "r"
        char readBuffer[65536];
        rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
        rapidjson::Document d;
        d.ParseStream(is);
        fclose(fp);

        if(d.HasParseError()){
            AV_ERROR("Error parsing nav meta file at path: {}", filePath);
            AV_ERROR(rapidjson::GetParseError_En(d.GetParseError()));
            return false;
        }

        //Begin parsing the file.
        using namespace rapidjson;

        Value::ConstMemberIterator itr;

        //Parse values.
        {
            itr = d.FindMember("Meshes");
            if(itr != d.MemberEnd() && itr->value.IsArray()){
                for(Value::ConstValueIterator memItr = itr->value.Begin(); memItr != itr->value.End(); ++memItr){
                    const rapidjson::Value& arrayVal = *memItr;
                    if(!arrayVal.IsObject()) continue;
                    MapNavMetaParserData meta;

                    Value::ConstMemberIterator itr = arrayVal.FindMember("name");
                    if(itr != arrayVal.MemberEnd()){
                        if(!itr->value.IsString()) return false;
                        const char* meshName = itr->value.GetString();
                        meta.meshName = meshName;
                    }else return false;

                    itr = arrayVal.FindMember("tileSize");
                    if(itr != arrayVal.MemberEnd()){
                        if(!itr->value.IsNumber()) return false;
                        meta.tileSize = itr->value.GetDouble();
                    }else return false;

                    itr = arrayVal.FindMember("cellSize");
                    if(itr != arrayVal.MemberEnd()){
                        if(!itr->value.IsNumber()) return false;
                        meta.cellSize = itr->value.GetDouble();
                    }else return false;

                    float tileSize = meta.tileSize * meta.cellSize;
                    float numTilesInChunk = SystemSettings::getWorldSlotSize() / tileSize;
                    meta.calculatedTileSize = tileSize;
                    meta.calculatedTilesInChunk = int(numTilesInChunk);

                    out.push_back(meta);
                }
            }
        }

        return true;
    }
}
