#include "RecipeNavMeshJob.h"

#include "Logger/Log.h"
#include "World/Slot/Recipe/RecipeData.h"
#include "System/SystemSetup/SystemSettings.h"

#include "World/Nav/Parser/DetourMeshBinaryParser.h"
#include <string>

#include "System/Util/PathUtils.h"

#include "DetourNavMesh.h"
#include <dirent.h>
#include <regex>

namespace AV{
    RecipeNavMeshJob::RecipeNavMeshJob(RecipeData *data)
    : mData(data){

    }

    void RecipeNavMeshJob::process(){
        //AV_INFO("Starting ogre recipe job! {}", mData->coord);

        _processFile();
    }

    void RecipeNavMeshJob::finish(){
        //AV_INFO("Finishing ogre recipe job! {}", mData->coord);

        mData->jobDoneCounter++;
    }

    void RecipeNavMeshJob::_getNavMeshId(const char* n, int* outNavId){
        const std::string name(n);
        size_t found = name.find_first_of("-");
        //Stringent checks here because if this is called it should have passed the regex.
        assert(found != std::string::npos);
        std::string first = name.substr(0, found);
        *outNavId = std::stoi(first);
    }

    bool RecipeNavMeshJob::_processFile(){
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if(!SystemSettings::isMapsDirectoryViable()) {
            AV_ERROR("There was an error processing nav mesh recipe job {}. The maps directory isn't viable.", mData->coord);
            return false;
        }

        /*DetourMeshBinaryParser parser;
        const std::string jsonFilePath = SystemSettings::getMapsDirectory() + "/" + mData->coord.getFilePath() + "/nav/navMeshData.json";
        if(!fileExists(jsonFilePath)){
            return false;
        }

        //Create the vector here if we've got this far.
        if(!mData->navMeshData){
            mData->navMeshData = new std::vector<NavMeshTileData>();
        }

        std::vector<NavMeshTileData>& outData = *(mData->navMeshData);
        outData.clear();
        bool success = parser.parseJsonMetaFile(jsonFilePath, outData);
        if(!success) return false;

        //TODO temporary while I decide what to do about multiple meshes.
        const std::string basePath = SystemSettings::getMapsDirectory() + "/" + mData->coord.getFilePath() + "/nav/";
        for(size_t i = 0; i < outData.size() && i < 1; i++){
            //Temporary for now.
            const std::string filePath = basePath + std::to_string(i) + ".nav";
            dtNavMesh* mesh = parser.parseFile(filePath);
            if(!mesh){
                AV_ERROR("Error parsing nav mesh {} with name {}.", i, outData[i].meshName);
                mData->navMeshData = 0;
                return false;
            }
            outData[i].mesh = mesh;
        }

        //Return true here because it might've just not found anything rather than failed.
        if(outData.size() == 0) return true;



        return true;*/

        if(!mData->navMeshData){
            mData->navMeshData = new std::vector<NavMeshTileData>();
        }
        const std::string basePath = SystemSettings::getMapsDirectory() + "/" + mData->coord.getFilePath() + "/nav/";
        DetourMeshBinaryParser binaryParser;

        //TODO Cross platform, ideally with c++ filesystem.
        DIR *d = opendir(basePath.c_str());
        size_t path_len = basePath.length();
        int r = -1;

        if (d){
            struct dirent *p;
            r = 0;
            while (!r && (p=readdir(d))){
                int r2 = -1;
                char *buf;
                size_t len;

                if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")){
                    continue;
                }

                static const std::regex floatRegex("^\\d+-\\d+.nav$");
                if(std::regex_match(p->d_name, floatRegex)){
                    //Try to load this file.

                    DetourMeshBinaryParser::TileData outData;
                    bool success = binaryParser.parseTile(basePath + p->d_name, &outData);
                    if(success){
                        int navId = 0;
                        _getNavMeshId(p->d_name, &navId);

                        dtMeshHeader* h = (dtMeshHeader*)outData.tileData;
                        //TODO Use the correct values here.
                        h->x += mData->coord.chunkX() * 2;
                        h->y += mData->coord.chunkY() * 2;

                        h->bmin[0] += mData->coord.chunkX() * 500;
                        h->bmin[2] += mData->coord.chunkY() * 500;
                        h->bmax[0] += mData->coord.chunkX() * 500;
                        h->bmax[2] += mData->coord.chunkY() * 500;

                        mData->navMeshData->push_back({
                            outData.tileData,
                            outData.dataSize,
                            0,
                            outData.x,
                            outData.y,
                            navId
                        });
                    }
                }
            }

            closedir(d);
        }

        return true;
    }
}
