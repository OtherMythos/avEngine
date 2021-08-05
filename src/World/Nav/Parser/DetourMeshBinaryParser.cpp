#include "DetourMeshBinaryParser.h"

#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "DetourNavMeshBuilder.h"

#include "Logger/Log.h"

#include <iostream>
#include <cstring>

#include "System/Util/PathUtils.h"

#include "rapidjson/document.h"
#include <rapidjson/filereadstream.h>
#include <rapidjson/error/en.h>

namespace AV{
    DetourMeshBinaryParser::DetourMeshBinaryParser(){

    }

    bool DetourMeshBinaryParser::parseJsonMetaFile(const std::string& filePath, std::vector<NavMeshTileData>& outVec) const{
        if(!fileExists(filePath)){
            return false;
        }

        outVec.clear();

        FILE* fp = fopen(filePath.c_str(), "r"); // non-Windows use "r"
        char readBuffer[65536];
        rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
        rapidjson::Document d;
        d.ParseStream(is);
        fclose(fp);

        if(d.HasParseError()){
            AV_ERROR("Error parsing nav mesh meta file at path: {}", filePath);
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

                    Value::ConstMemberIterator nameItr = arrayVal.FindMember("name");
                    if(nameItr != arrayVal.MemberEnd()){
                        if(!nameItr->value.IsString()) return false;
                        const char* meshName = nameItr->value.GetString();
                        //outVec.push_back( {meshName, 0} );
                    }

                }
            }
        }

        return true;
    }

    dtNavMesh* DetourMeshBinaryParser::parseFile(const std::string& filePath){
        if(!fileExists(filePath.c_str())) return 0;

        return _loadAll(filePath.c_str());
    }

    static const int NAVMESHSET_MAGIC = 'M'<<24 | 'S'<<16 | 'E'<<8 | 'T'; //'MSET';
    static const int NAVMESHSET_VERSION = 1;

    struct NavMeshSetHeader
    {
        int magic;
        int version;
        int numTiles;
        dtNavMeshParams params;
    };

    struct NavMeshTileHeader
    {
        dtTileRef tileRef;
        int dataSize;
    };

    dtNavMesh* DetourMeshBinaryParser::_loadAll(const char* path) const{
        FILE* fp = fopen(path, "rb");
        if (!fp) return 0;

        // Read header.
        NavMeshSetHeader header;
        size_t readLen = fread(&header, sizeof(NavMeshSetHeader), 1, fp);
        if (readLen != 1)
        {
            fclose(fp);
            return 0;
        }
        if (header.magic != NAVMESHSET_MAGIC)
        {
            fclose(fp);
            return 0;
        }
        if (header.version != NAVMESHSET_VERSION)
        {
            fclose(fp);
            return 0;
        }

        dtNavMesh* mesh = dtAllocNavMesh();
        if (!mesh)
        {
            fclose(fp);
            return 0;
        }
        dtStatus status = mesh->init(&header.params);
        if (dtStatusFailed(status))
        {
            fclose(fp);
            return 0;
        }

        // Read tiles.
        for (int i = 0; i < header.numTiles; ++i)
        {
            NavMeshTileHeader tileHeader;
            readLen = fread(&tileHeader, sizeof(tileHeader), 1, fp);
            if (readLen != 1)
            {
                fclose(fp);
                return 0;
            }

            if (!tileHeader.tileRef || !tileHeader.dataSize)
                break;

            unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
            if (!data) break;
            memset(data, 0, tileHeader.dataSize);
            readLen = fread(data, tileHeader.dataSize, 1, fp);
            if (readLen != 1)
            {
                dtFree(data);
                fclose(fp);
                return 0;
            }

            mesh->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
        }

        fclose(fp);

        return mesh;
    }

    bool DetourMeshBinaryParser::parseTile(const std::string& filePath, TileData* out) const{
        FILE* fp = fopen(filePath.c_str(), "rb");
        if (!fp) return false;

        NavMeshTileHeader tileHeader;
        size_t readLen = fread(&tileHeader, sizeof(tileHeader), 1, fp);
        if (readLen != 1)
        {
            fclose(fp);
            return false;
        }

        if (!tileHeader.tileRef || !tileHeader.dataSize)
            return false;

        unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
        if (!data) return false;
        memset(data, 0, tileHeader.dataSize);
        readLen = fread(data, tileHeader.dataSize, 1, fp);
        if (readLen != 1)
        {
            dtFree(data);
            fclose(fp);
            return false;
        }

        // unsigned char* thing = (unsigned char*)malloc(tileHeader.dataSize);
        // memcpy(thing, data, tileHeader.dataSize);

        out->dataSize = tileHeader.dataSize;
        out->tileData = data;
        //out->tileData = thing;

        dtMeshHeader* h = (dtMeshHeader*)data;
        out->x = h->x;
        out->y = h->y;

        //mesh->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
        return true;
    }
}
